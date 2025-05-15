# Multi-Threaded Analysis
This repository contains implementations and analysis for parallel statistical computations on large datasets (1GB+) using pthreads, with performance comparisons between thread affinity and non-affinity approaches. The assignment focuses on:
- Task 3: Parallel term frequency analysis of multilingual UN documents (100,535 files).
- Task 5: Parallel matrix operations on traffic sensor data (35,040 × 8,600 matrix).

## File Structure
```
├── 22i-0977_A_A1_task3_aff.c       # Task 3 (with thread affinity)
├── 22i-0977_A_A1_task3_no_aff.c    # Task 3 (without affinity)
├── 22i-0977_A_A1_task5_aff.c       # Task 5 (with thread affinity)
├── 22i-0977_A_A1_task5_noaff.c     # Task 5 (without affinity)
├── extract.py                       # Data extraction utility
├── process.py                       # Data preprocessing script
├── process_ca_his.ipynb             # Jupyter notebook for traffic data
├── process_ca_txt.ipynb             # Jupyter notebook for text data
├── i220977_A_A1_report.pdf          # Full report (performance analysis)
└── .gitattributes                  # Git configuration
```

## Compile & Run
``` bash
# Task 3 (Term Frequency Analysis)
gcc 22i-0977_A_A1_task3_aff.c -o task3_aff -lpthread
./task3_aff

# Task 5 (Matrix Operations)
gcc 22i-0977_A_A1_task5_aff.c -o task5_aff -lpthread
./task5_aff
```

## Key Features
### Task 3: Multilingual UN Documents
- Parallel file processing: 100,535 files distributed cyclically across threads.
- Fine-grained locking: Hash table with per-bucket mutexes for concurrent word counting.

- Optimizations:
Dynamic file fetching by threads.
Reduced contention via hash bucket-level synchronization.

### Task 5: Traffic Sensor Data Matrix
- Parallel matrix operations: Sum, max/min, row/column sums.
- Thread affinity experiments: Compare pthread_setaffinity_np() vs OS scheduling.

## Notes  
### **Dataset Sources**:  
- **Task 5**: [LargeST Traffic Dataset](https://arxiv.org/abs/2106.09320)  
- **Task 3**: [MultiUN Corpus](https://conferences.unite.un.org/UNCORPUS/en)  
- Preprocessing: Use Jupyter notebooks (process_*.ipynb) to format raw data.