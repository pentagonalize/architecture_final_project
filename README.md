# architecture_final_project

# gem5 Docker Environment

Run gem5 simulations in a ready-to-go Docker container.

## Usage

```bash
# 1. Pull the container
docker pull danzi13/gem5-env

# 2. Run it
docker run -it danzi13/gem5-env

# 3. Inside the container
cd /gem5

# 4. Run your binary (replace 'your_binary' with your executable)
build/X86/gem5.opt configs/deprecated/example/se.py --cmd=./your_binary

# 5. View stats
cat m5out/stats
