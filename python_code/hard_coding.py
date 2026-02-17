import numpy as np

mean = np.load('scaler_mean.npy')
std  = np.load('scaler_std.npy')

print("mean:")
for i, v in enumerate(mean):
    print(f"  [{i}] {v:.6f}f,")

print("\nstd:")
for i, v in enumerate(std):
    print(f"  [{i}] {v:.6f}f,")
