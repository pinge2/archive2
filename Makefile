PY := py -3.11

all:
	@clear
	@$(PY) main.py


pip:
	@$(PY) -m pip install torch torchvision numpy pygame pymunk pybullet