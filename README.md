# MiMMO

MiMMO (Minimal Memory Manager for Openacc) is a simple, safe, easy to use CPU/GPU memory manager to work
with OpenACC.

The pragma-based approach of OpenACC is extremely useful, however it only allows for a very high-level
management of host and device memory, and it is often confusing for those who like to understand what is
happening behind the curtain.

MiMMO aims to allow the users to have a finer control over memory allocations and movements, while keeping
it simple and safe.

Under the hood, MiMMO uses the OpenACC API; however, it hides it completely, allowing the user to write
clean code for both host and device.

The manager also keeps track of all memory allocations on host and device, and reports can be requested at
any time.
