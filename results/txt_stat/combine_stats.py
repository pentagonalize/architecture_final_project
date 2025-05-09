import os
import pandas as pd

data = []
base_dir = "outputs"

for filename in os.listdir(base_dir):
    if filename.endswith("_tinyStats.txt"):
        config_name = filename.replace("_tinyStats.txt", "")
        filepath = os.path.join(base_dir, filename)
        
        with open(filepath, "r") as file:
            row = {"Config": config_name}
            category = None
            for line in file:
                line = line.strip()
                if not line:
                    continue
                if not ':' in line:
                    # category header like "CPU Performance:"
                    category = line.rstrip(":")
                else:
                    key, val = line.split(":", 1)
                    val = val.strip()
                    try:
                        val = float(val)
                    except ValueError:
                        pass  # keep as string if not numeric
                    row[f"{category}::{key.strip()}"] = val
            data.append(row)

# Create the DataFrame
df = pd.DataFrame(data)

# Optional: save to CSV
df.to_csv("combined_stats.csv", index=False)

