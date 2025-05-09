import os
import re

# Define fields to extract by category
FIELDS = {
    "Sim Time": [
        "simSeconds", "simTicks", "simFreq"
    ],
    "CPU Performance": [
        "system.cpu.numCycles",
        "system.cpu.cpi", "system.cpu.ipc",
        "system.cpu.commitStats0.numInsts",
        "system.cpu.commitStats0.cpi",
        "system.cpu.commitStats0.numMemRefs",
        "system.cpu.commitStats0.numLoadInsts",
        "system.cpu.commitStats0.numStoreInsts",
        "system.cpu.fetchStats0.fetchRate",
        "system.cpu.executeStats0.instRate"
    ],
    "Memory Access Timing": [
        "system.mem_ctrls.dram.numReads::total",
        "system.mem_ctrls.dram.numWrites::total",
        "system.mem_ctrls.dram.totQLat",
        "system.mem_ctrls.dram.totBusLat",
        "system.mem_ctrls.dram.totMemAccLat",
        "system.mem_ctrls.dram.avgQLat",
        "system.mem_ctrls.dram.avgBusLat",
        "system.mem_ctrls.dram.avgMemAccLat"
    ],
    "Row Buffer Hits": [
        "system.mem_ctrls.dram.readRowHits",
        "system.mem_ctrls.dram.writeRowHits",
        "system.mem_ctrls.dram.readRowHitRate",
        "system.mem_ctrls.dram.writeRowHitRate"
    ],
    "Total + Component Energy Usage": [
        "system.mem_ctrls.dram.rank0.totalEnergy",
        "system.mem_ctrls.dram.rank1.totalEnergy",
        "system.mem_ctrls.dram.rank0.actEnergy",
        "system.mem_ctrls.dram.rank0.preEnergy",
        "system.mem_ctrls.dram.rank0.readEnergy",
        "system.mem_ctrls.dram.rank0.writeEnergy",
        "system.mem_ctrls.dram.rank1.actEnergy",
        "system.mem_ctrls.dram.rank1.preEnergy",
        "system.mem_ctrls.dram.rank1.readEnergy",
        "system.mem_ctrls.dram.rank1.writeEnergy"
    ],
    "Power Metrics": [
        "system.mem_ctrls.dram.rank0.averagePower",
        "system.mem_ctrls.dram.rank1.averagePower"
    ],
    "Energy Breakdown": [
        "system.mem_ctrls.dram.rank0.actBackEnergy",
        "system.mem_ctrls.dram.rank0.preBackEnergy",
        "system.mem_ctrls.dram.rank0.actPowerDownEnergy",
        "system.mem_ctrls.dram.rank0.prePowerDownEnergy",
        "system.mem_ctrls.dram.rank0.pwrStateTime::IDLE",
        "system.mem_ctrls.dram.rank1.actBackEnergy",
        "system.mem_ctrls.dram.rank1.preBackEnergy",
        "system.mem_ctrls.dram.rank1.actPowerDownEnergy",
        "system.mem_ctrls.dram.rank1.prePowerDownEnergy",
        "system.mem_ctrls.dram.rank1.pwrStateTime::IDLE"
    ],
    "Bus/Memory Throughput": [
        "system.mem_ctrls.dram.bwTotal::total",
        "system.mem_ctrls.dram.avgRdBW",
        "system.mem_ctrls.dram.avgWrBW",
        "system.mem_ctrls.dram.peakBW",
        "system.mem_ctrls.dram.busUtil"
    ]
}

# Ensure output folder exists
os.makedirs("outputs", exist_ok=True)

# Scan .txt files in current directory
for filename in os.listdir():
    if filename.endswith(".txt") and not filename.startswith("outputs"):
        stats = {key: {} for key in FIELDS}
        with open(filename, "r") as file:
            content = file.read()

            for category, keys in FIELDS.items():
                for key in keys:
                    match = re.search(rf"^{re.escape(key)}\s+([^\s#]+)", content, re.MULTILINE)
                    if match:
                        stats[category][key] = match.group(1)

        # Output concatenated summary
        tiny_stats_filename = f"outputs/{filename.replace('.txt', '')}_tinyStats.txt"
        with open(tiny_stats_filename, "w") as out:
            for category, fields in stats.items():
                out.write(f"{category}:\n")
                for k, v in fields.items():
                    out.write(f"{k}: {v}\n")
                out.write("\n")
