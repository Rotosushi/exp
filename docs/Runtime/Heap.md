
# Heap Allocation

I want to write a heap implementation. for the runtime of the language. 
This is a large and complex task. big enough to be a project on it's own i'd wager.

* note: from copilot *
TLSF (Two-Level Segregate Fit) is a dynamic memory allocation strategy designed for real-time systems where predictable and efficient memory allocation is critical. It provides constant-time allocation and deallocation while minimizing fragmentation. Here's a detailed explanation:

How TLSF Works
TLSF divides memory into blocks and organizes them into two levels of segregated free lists:

First-Level Segregation:

Memory blocks are grouped into size classes based on their most significant bit (MSB). For example, blocks of size 16–31 bytes might belong to one class, 32–63 bytes to another, and so on.
This ensures that blocks of similar sizes are grouped together, reducing fragmentation.

Second-Level Segregation:

Within each first-level size class, blocks are further divided into finer-grained size ranges. This allows for more precise allocation and reduces wasted space.
For example, the 32–63 byte class might be divided into 32–39, 40–47, 48–55, and 56–63 byte ranges.

Bitmaps for Fast Lookup:

TLSF uses bitmaps to track which size classes and ranges have free blocks. This allows the allocator to quickly find a suitable block in constant time.

Splitting and Coalescing:

When a block is allocated, it may be split into smaller blocks if the requested size is smaller than the available block.
When a block is freed, it is coalesced with adjacent free blocks to reduce fragmentation.

Key Features
Constant-Time Operations: Allocation and deallocation are performed in O(1) time, making TLSF suitable for real-time systems.

Low Fragmentation: The two-level segregation and coalescing mechanisms minimize both internal and external fragmentation.

Predictability: TLSF provides deterministic behavior, which is critical for systems with strict timing requirements.

Advantages
Real-Time Guarantees: TLSF is designed for systems where predictable allocation times are essential, such as embedded systems, robotics, and gaming.

Efficient Memory Usage: By segregating blocks and coalescing free memory, TLSF reduces wasted space.
Scalability: TLSF works well across a wide range of allocation sizes.

Disadvantages
Complexity: TLSF is more complex to implement compared to simpler strategies like first fit or best fit.
Overhead: The use of bitmaps and segregated lists introduces some memory overhead.

Use Cases
Embedded Systems: TLSF is widely used in embedded systems where memory is limited, and real-time performance is critical.
Real-Time Operating Systems (RTOS): Many RTOS implementations use TLSF for their dynamic memory allocation needs.
Gaming Engines: Games often require fast and predictable memory allocation, making TLSF a good fit.

