# Cache Test Raw Output

No Cache:

```NULL
>>>>> Put Delay Test <<<<<
0...100...200...300...400...500...600...700...800...900...
Average Delay For Different Size Data:
0 ~ 0.25MB     : 2.14ms throughput: 467.32/s
0.25MB ~ 0.5MB : 17.75ms throughput: 56.34/s
0.5MB ~ 1MB    : 29.36ms throughput: 34.06/s
1MB ~ 1.5MB    : 41.09ms throughput: 24.34/s
1.5MB ~ 2MB    : 37.99ms throughput: 26.32/s

Average Delay: 28.66ms, Average Throughput: 34.90/s
Total Size Insert: 956MB

>>>>> Get Delay Test <<<<<
Average Delay And Throughput For Different Size Data:
0 ~ 0.25MB     : 1437.15ms throughput: 695.82/s
0.25MB ~ 0.5MB : 1374.00ms throughput: 727.80/s
0.5MB ~ 1MB    : 1522.52ms throughput: 656.81/s
1MB ~ 1.5MB    : 1327.18ms throughput: 753.48/s
1.5MB ~ 2MB    : 1074.67ms throughput: 930.52/s

Average Delay: 1341.41µs, Average Throughput: 745.49/s

>>>>> Delete Delay Test <<<<<
Average Delay And Throughput For Different Size Data:
0 ~ 0.25MB     : 1425.53ms throughput: 701.49/s
0.25MB ~ 0.5MB : 1384.01ms throughput: 722.54/s
0.5MB ~ 1MB    : 1539.83ms throughput: 649.42/s
1MB ~ 1.5MB    : 1339.20ms throughput: 746.71/s
1.5MB ~ 2MB    : 1096.62ms throughput: 911.89/s

Average Delay: 1353.41µs, Average Throughput: 738.88/s

>>>>> Summary <<<<<
It takes 31s to complete the test.
```

Index Cached:

```NULL

```
