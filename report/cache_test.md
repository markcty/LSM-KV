No cache:

```
>>>>> Put Delay Test <<<<<
0...100...200...300...400...500...600...700...800...900...
Average Delay For Different Size Data:
0 ~ 0.25MB     : 0.80ms throughput: 1242.72/s
0.25MB ~ 0.5MB : 10.47ms throughput: 95.48/s
0.5MB ~ 1MB    : 33.67ms throughput: 29.70/s
1MB ~ 1.5MB    : 37.33ms throughput: 26.79/s
1.5MB ~ 2MB    : 52.42ms throughput: 19.08/s

Average Delay: 31.83ms, Average Throughput: 31.42/s
Total Size Insert: 980MB

>>>>> Get Delay Test <<<<<
Average Delay And Throughput For Different Size Data:
0 ~ 0.25MB     : 1743.03µs throughput: 573.71/s
0.25MB ~ 0.5MB : 1809.50µs throughput: 552.64/s
0.5MB ~ 1MB    : 1562.03µs throughput: 640.19/s
1MB ~ 1.5MB    : 1392.24µs throughput: 718.27/s
1.5MB ~ 2MB    : 1147.29µs throughput: 871.62/s

Average Delay: 1475.47µs, Average Throughput: 677.75/s

>>>>> Delete Delay Test <<<<<
Average Delay And Throughput For Different Size Data:
0 ~ 0.25MB     : 1747.07µs throughput: 572.39/s
0.25MB ~ 0.5MB : 1811.99µs throughput: 551.88/s
0.5MB ~ 1MB    : 1597.63µs throughput: 625.93/s
1MB ~ 1.5MB    : 1452.51µs throughput: 688.46/s
1.5MB ~ 2MB    : 1214.60µs throughput: 823.32/s

Average Delay: 1516.44µs, Average Throughput: 659.44/s

>>>>> Summary <<<<<
It takes 35s to complete the test.
```

Index cached:

```
>>>>> Put Delay Test <<<<<
0...100...200...300...400...500...600...700...800...900...
Average Delay For Different Size Data:
0 ~ 0.25MB     : 1.50ms throughput: 665.18/s
0.25MB ~ 0.5MB : 10.97ms throughput: 91.14/s
0.5MB ~ 1MB    : 31.80ms throughput: 31.45/s
1MB ~ 1.5MB    : 51.55ms throughput: 19.40/s
1.5MB ~ 2MB    : 52.15ms throughput: 19.18/s

Average Delay: 34.97ms, Average Throughput: 28.59/s
Total Size Insert: 979MB

>>>>> Get Delay Test <<<<<
Average Delay And Throughput For Different Size Data:
0 ~ 0.25MB     : 139.55µs throughput: 7165.87/s
0.25MB ~ 0.5MB : 177.97µs throughput: 5618.85/s
0.5MB ~ 1MB    : 205.07µs throughput: 4876.50/s
1MB ~ 1.5MB    : 267.71µs throughput: 3735.40/s
1.5MB ~ 2MB    : 319.73µs throughput: 3127.66/s

Average Delay: 235.92µs, Average Throughput: 4238.78/s

>>>>> Delete Delay Test <<<<<
Average Delay And Throughput For Different Size Data:
0 ~ 0.25MB     : 142.96µs throughput: 6994.98/s
0.25MB ~ 0.5MB : 188.52µs throughput: 5304.39/s
0.5MB ~ 1MB    : 225.19µs throughput: 4440.74/s
1MB ~ 1.5MB    : 302.53µs throughput: 3305.40/s
1.5MB ~ 2MB    : 357.74µs throughput: 2795.32/s

Average Delay: 260.57µs, Average Throughput: 3837.67/s

>>>>> Summary <<<<<
It takes 36s to complete the test.
```
