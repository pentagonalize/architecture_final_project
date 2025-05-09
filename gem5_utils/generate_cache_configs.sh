#!/bin/bash

# Script to generate gem5 config files with different cache configurations
# Cache sizes: L1 [16kB, 32kB, 64kB], L2 [32kB, 64kB, 128kB]
# Cache associativity: [2, 4, 8]

# Create output directory if it doesn't exist
mkdir -p cache_configs

# Function to generate a config file with specific cache parameters
generate_config() {
    local l1_size="$1"
    local l2_size="$2"
    local assoc="$3"
    local filename="cache_configs/config_L1_${l1_size}_L2_${l2_size}_assoc_${assoc}.py"
    
    cat > "$filename" << EOF
import m5
from m5.objects import *
from m5.objects import Cache
import argparse

class L1Cache(Cache):
    assoc = ${assoc}
    tag_latency = 2
    data_latency = 2
    response_latency = 2
    mshrs = 4
    tgts_per_mshr = 20

    def connectCPU(self, cpu):
        # need to define this in a base class!
        raise NotImplementedError

    def connectBus(self, bus):
        self.mem_side = bus.cpu_side_ports

class L1ICache(L1Cache):
    size = '${l1_size}kB'

    def connectCPU(self, cpu):
        self.cpu_side = cpu.icache_port

class L1DCache(L1Cache):
    size = '${l1_size}kB'

    def connectCPU(self, cpu):
        self.cpu_side = cpu.dcache_port

class L2Cache(Cache):
    size = '${l2_size}kB'
    assoc = ${assoc}
    tag_latency = 20
    data_latency = 20
    response_latency = 20
    mshrs = 20
    tgts_per_mshr = 12

    def connectCPUSideBus(self, bus):
        self.cpu_side = bus.mem_side_ports

    def connectMemSideBus(self, bus):
        self.mem_side = bus.cpu_side_ports

# Parse command-line arguments
parser = argparse.ArgumentParser(description="gem5 simulation script")
parser.add_argument("binary", help="Path to the binary to execute")
args = parser.parse_args()

system = System()

system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode = 'timing'
system.mem_ranges = [AddrRange('512MB')]

system.cpu = X86TimingSimpleCPU()
system.cpu.icache = L1ICache()
system.cpu.dcache = L1DCache()

system.membus = SystemXBar()

# Connect the CPU's cache ports to the caches
system.cpu.icache.connectCPU(system.cpu)
system.cpu.dcache.connectCPU(system.cpu)

# Set up the memory bus
system.l2bus = L2XBar()

system.cpu.icache.connectBus(system.l2bus)
system.cpu.dcache.connectBus(system.l2bus)

# Connect the L2 cache to the memory bus
system.l2cache = L2Cache()
system.l2cache.connectCPUSideBus(system.l2bus)
system.membus = SystemXBar()
system.l2cache.connectMemSideBus(system.membus)

# Set up the interrupt controller for X86
system.cpu.createInterruptController()
system.cpu.interrupts[0].pio = system.membus.mem_side_ports
system.cpu.interrupts[0].int_requestor = system.membus.cpu_side_ports
system.cpu.interrupts[0].int_responder = system.membus.mem_side_ports

system.system_port = system.membus.cpu_side_ports

# Create a memory controller
system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

binary = args.binary

# For gem5 V21 and beyond
system.workload = SEWorkload.init_compatible(binary)

# Create a process to run the binary
process = Process()
process.cmd = [binary]
system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system = False, system = system)
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()

print('Exiting @ tick {} because {}'
.format(m5.curTick(), exit_event.getCause()))
EOF

    echo "Generated config file: $filename"
}

echo "Generating cache configuration files..."

# Define cache size combinations
declare -a l1_sizes=("16" "32" "64")
declare -a l2_sizes=("32" "64" "128")
declare -a associativities=("2" "4" "8")

# Generate all combinations
for i in "${!l1_sizes[@]}"; do
    l1_size="${l1_sizes[$i]}"
    l2_size="${l2_sizes[$i]}"
    
    for assoc in "${associativities[@]}"; do
        generate_config "$l1_size" "$l2_size" "$assoc"
    done
done

echo "Done! Generated $(( ${#l1_sizes[@]} * ${#associativities[@]} )) configuration files."
