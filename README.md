# MIMMO

MIMMO (MInimal Memory Manager for Openacc) is a simple memory manager to work with OpenACC.

The pragma-based approach of OpenACC is extremely useful, however it only allows for a very high-level
management of host and device memory, and it is often confusing for those who like to understand what is
happening behind the curtain.

MIMMO aims to allow the users to have a finer control over memory allocations and movements, while keeping
it simple and safe.

Under the hood, MIMMO uses the OpenACC API; however, it hides it completely, allowing the user to write
clean code for both host and device.
