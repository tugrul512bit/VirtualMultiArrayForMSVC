# VirtualMultiArrayForMSVC

This is MSVC-version of: https://github.com/tugrul512bit/VirtualMultiArray/wiki

# Virtual Memory System For Arrays With Getters and Setters, in C++

- Requires OpenCL for data transmissions between graphics cards & RAM, Vcpkg to install OpenCL.
- Getters and setters use OpenCL to get/set data on graphics cards with seamless indexing
- Data is cached in RAM with faster access for redundant accesses
- When cache doesn't hold new data, some old data is paged out to graphics cards to make space in cache (LRU approximation)

Advantages:

- RAM only holds cache part which is small, array is mainly kept on graphics cards: system is responsive even when using an array bigger than RAM
- data is shared between graphics cards so a single array can be bigger than a single graphics card's memory
- if data re-use ratio is high, then CPU's caches further improve performance

Disadvantages:

- small objects are not efficient because of PCIE latency. 8-byte elements can have only 200-300 MB/s random-access performance while 60kB elements can reach 50-60 GB/s
- if graphics cards are not identical then it is harder to fine-tune total bandwidth & capacity at the same time

```C++
// Object is an object with plain old data (any size from 1 byte to 1 megabyte per element)
VirtualMultiArray<Object> test(n, GraphicsCardSupplyDepot().requestGpus(), pageSize, pagesPerLRU, numLRU);
test.set(400,Object()); // written to cache
auto t1 = test.get(400);          // cache-hit
auto t2 = test.get(500);          // cache-miss ----> comes from graphics card memory (so you can use 48GB memory of an two RTX4090s as an automatic storage of "Object"s)
test.prefetch(401);               // prepare asynchronously for next iteration
Object findThis;
findThis.value = 5;
auto foundIndex = test.find(&findThis, &findThis.value,100);  // find objects with given field value, within whole array (in cache, in graphics cards), maximum 100 indices will be listed
auto t3 = test.getUncached(500);                              // bypass cache and directly stream data from graphics cards

// memory-mapped access with reservation of indices between 303 and 804, with a lambda function given by user to compute/access all elements at once, fast
bool read=true;
bool write=true;
bool pinned=false;
test.mappedReadWriteAccess(303,501,[](int * buf){
	// even with index = 303 as starting point, buf is 4096-aligned for performance
	// do something with elements between index=303 & 804
	for(int i=303;i<303+501;i++)
		buf[i]=i;
},pinned,read,write);
// memory-mapped access does sequential block processing until whole region (303-501) is processed, in-order, without deadlock, using raw arrays for higher read/write/compute performance
```

Benchmark for Ryzen 7900 + RTX4070 (pcie v4.0 x16) + RTX4060ti (pcie v4.0 x4):
```
64-thread random-access performance benchmark.
Element size = 61440 bytes
Array size = 4608000000 bytes
Cache size = 46080000 bytes
init...
...complete
hit-rate=1.000013%: 938910100 nanoseconds     (bandwidth = 523.50 MB/s)
hit-rate=1.052646%: 858268700 nanoseconds     (bandwidth = 572.69 MB/s)
hit-rate=1.108048%: 873965600 nanoseconds     (bandwidth = 562.40 MB/s)
hit-rate=1.166366%: 898255200 nanoseconds     (bandwidth = 547.19 MB/s)
hit-rate=1.227754%: 896545100 nanoseconds     (bandwidth = 548.24 MB/s)
hit-rate=1.292373%: 907532900 nanoseconds     (bandwidth = 541.60 MB/s)
hit-rate=1.360392%: 908988200 nanoseconds     (bandwidth = 540.73 MB/s)
hit-rate=1.431992%: 936818900 nanoseconds     (bandwidth = 524.67 MB/s)
hit-rate=1.507360%: 917594200 nanoseconds     (bandwidth = 535.66 MB/s)
hit-rate=1.586695%: 920446600 nanoseconds     (bandwidth = 534.00 MB/s)
hit-rate=1.670205%: 928024900 nanoseconds     (bandwidth = 529.64 MB/s)
hit-rate=1.758110%: 925015900 nanoseconds     (bandwidth = 531.36 MB/s)
hit-rate=1.850642%: 942879200 nanoseconds     (bandwidth = 521.30 MB/s)
hit-rate=1.948045%: 933953900 nanoseconds     (bandwidth = 526.28 MB/s)
hit-rate=2.050573%: 928207800 nanoseconds     (bandwidth = 529.54 MB/s)
hit-rate=2.158498%: 943351700 nanoseconds     (bandwidth = 521.04 MB/s)
hit-rate=2.272103%: 929200000 nanoseconds     (bandwidth = 528.97 MB/s)
hit-rate=2.391688%: 927367300 nanoseconds     (bandwidth = 530.02 MB/s)
hit-rate=2.517566%: 885818300 nanoseconds     (bandwidth = 554.88 MB/s)
hit-rate=2.650070%: 875700000 nanoseconds     (bandwidth = 561.29 MB/s)
hit-rate=2.789547%: 876834400 nanoseconds     (bandwidth = 560.56 MB/s)
hit-rate=2.936365%: 878872400 nanoseconds     (bandwidth = 559.26 MB/s)
hit-rate=3.090911%: 860890200 nanoseconds     (bandwidth = 570.94 MB/s)
hit-rate=3.253590%: 862838000 nanoseconds     (bandwidth = 569.66 MB/s)
hit-rate=3.424832%: 840444600 nanoseconds     (bandwidth = 584.83 MB/s)
hit-rate=3.605086%: 847507200 nanoseconds     (bandwidth = 579.96 MB/s)
hit-rate=3.794828%: 835695400 nanoseconds     (bandwidth = 588.16 MB/s)
hit-rate=3.994555%: 846823300 nanoseconds     (bandwidth = 580.43 MB/s)
hit-rate=4.204795%: 853669400 nanoseconds     (bandwidth = 575.77 MB/s)
hit-rate=4.426100%: 830076200 nanoseconds     (bandwidth = 592.14 MB/s)
hit-rate=4.659053%: 817809100 nanoseconds     (bandwidth = 601.02 MB/s)
hit-rate=4.904266%: 821257200 nanoseconds     (bandwidth = 598.50 MB/s)
hit-rate=5.162385%: 808038700 nanoseconds     (bandwidth = 608.29 MB/s)
hit-rate=5.434090%: 793853100 nanoseconds     (bandwidth = 619.16 MB/s)
hit-rate=5.720095%: 799862800 nanoseconds     (bandwidth = 614.51 MB/s)
hit-rate=6.021152%: 798443900 nanoseconds     (bandwidth = 615.60 MB/s)
hit-rate=6.338055%: 788965400 nanoseconds     (bandwidth = 622.99 MB/s)
hit-rate=6.671637%: 790429900 nanoseconds     (bandwidth = 621.84 MB/s)
hit-rate=7.022776%: 804718900 nanoseconds     (bandwidth = 610.80 MB/s)
hit-rate=7.392395%: 797437200 nanoseconds     (bandwidth = 616.37 MB/s)
hit-rate=7.781469%: 791176700 nanoseconds     (bandwidth = 621.25 MB/s)
hit-rate=8.191020%: 809115200 nanoseconds     (bandwidth = 607.48 MB/s)
hit-rate=8.622126%: 797291100 nanoseconds     (bandwidth = 616.49 MB/s)
hit-rate=9.075922%: 804411300 nanoseconds     (bandwidth = 611.03 MB/s)
hit-rate=9.553602%: 783233600 nanoseconds     (bandwidth = 627.55 MB/s)
hit-rate=10.056423%: 774702000 nanoseconds     (bandwidth = 634.46 MB/s)
hit-rate=10.585709%: 780436200 nanoseconds     (bandwidth = 629.80 MB/s)
hit-rate=11.142851%: 768790100 nanoseconds     (bandwidth = 639.34 MB/s)
hit-rate=11.729317%: 755830600 nanoseconds     (bandwidth = 650.30 MB/s)
hit-rate=12.346650%: 765344600 nanoseconds     (bandwidth = 642.22 MB/s)
hit-rate=12.996474%: 745391700 nanoseconds     (bandwidth = 659.41 MB/s)
hit-rate=13.680498%: 746827900 nanoseconds     (bandwidth = 658.14 MB/s)
hit-rate=14.400525%: 715588800 nanoseconds     (bandwidth = 686.87 MB/s)
hit-rate=15.158447%: 719057700 nanoseconds     (bandwidth = 683.56 MB/s)
hit-rate=15.956260%: 714265500 nanoseconds     (bandwidth = 688.15 MB/s)
hit-rate=16.796063%: 684167900 nanoseconds     (bandwidth = 718.42 MB/s)
hit-rate=17.680067%: 690995700 nanoseconds     (bandwidth = 711.32 MB/s)
hit-rate=18.610596%: 683210000 nanoseconds     (bandwidth = 719.43 MB/s)
hit-rate=19.590101%: 654663000 nanoseconds     (bandwidth = 750.80 MB/s)
hit-rate=20.621159%: 647172100 nanoseconds     (bandwidth = 759.49 MB/s)
hit-rate=21.706484%: 648903400 nanoseconds     (bandwidth = 757.46 MB/s)
hit-rate=22.848930%: 648178600 nanoseconds     (bandwidth = 758.31 MB/s)
hit-rate=24.051505%: 628365200 nanoseconds     (bandwidth = 782.22 MB/s)
hit-rate=25.317374%: 596662500 nanoseconds     (bandwidth = 823.78 MB/s)
hit-rate=26.649867%: 584780400 nanoseconds     (bandwidth = 840.52 MB/s)
hit-rate=28.052492%: 559528600 nanoseconds     (bandwidth = 878.45 MB/s)
hit-rate=29.528939%: 526175200 nanoseconds     (bandwidth = 934.14 MB/s)
hit-rate=31.083094%: 528508600 nanoseconds     (bandwidth = 930.01 MB/s)
hit-rate=32.719046%: 522139800 nanoseconds     (bandwidth = 941.36 MB/s)
hit-rate=34.441101%: 470688400 nanoseconds     (bandwidth = 1044.26 MB/s)
hit-rate=36.253790%: 439588400 nanoseconds     (bandwidth = 1118.14 MB/s)
hit-rate=38.161885%: 419541400 nanoseconds     (bandwidth = 1171.56 MB/s)
hit-rate=40.170405%: 387833700 nanoseconds     (bandwidth = 1267.35 MB/s)
hit-rate=42.284637%: 364617100 nanoseconds     (bandwidth = 1348.04 MB/s)
hit-rate=44.510144%: 320094200 nanoseconds     (bandwidth = 1535.55 MB/s)
hit-rate=46.852783%: 291023300 nanoseconds     (bandwidth = 1688.94 MB/s)
hit-rate=49.318719%: 257959900 nanoseconds     (bandwidth = 1905.41 MB/s)
hit-rate=51.914441%: 218987000 nanoseconds     (bandwidth = 2244.52 MB/s)
hit-rate=54.646780%: 175845900 nanoseconds     (bandwidth = 2795.17 MB/s)
hit-rate=57.522926%: 149532600 nanoseconds     (bandwidth = 3287.04 MB/s)
hit-rate=60.550449%: 109833800 nanoseconds     (bandwidth = 4475.13 MB/s)
hit-rate=63.737315%: 63942100 nanoseconds     (bandwidth = 7686.95 MB/s)
hit-rate=67.091910%: 30220000 nanoseconds     (bandwidth = 16264.73 MB/s)
hit-rate=70.623063%: 17800600 nanoseconds     (bandwidth = 27612.55 MB/s)
hit-rate=74.340067%: 17707500 nanoseconds     (bandwidth = 27757.73 MB/s)
hit-rate=78.252702%: 16581500 nanoseconds     (bandwidth = 29642.67 MB/s)
hit-rate=82.371265%: 16330000 nanoseconds     (bandwidth = 30099.20 MB/s)
hit-rate=86.706595%: 16065700 nanoseconds     (bandwidth = 30594.37 MB/s)
hit-rate=91.270100%: 16245700 nanoseconds     (bandwidth = 30255.39 MB/s)
hit-rate=96.073789%: 15777300 nanoseconds     (bandwidth = 31153.62 MB/s)
cache size=1.011303x of data set: 15722400 nanoseconds     (bandwidth = 31262.40 MB/s)
cache size=1.064530x of data set: 15652900 nanoseconds     (bandwidth = 31401.21 MB/s)
cache size=1.120557x of data set: 14200100 nanoseconds     (bandwidth = 34613.84 MB/s)
cache size=1.179534x of data set: 14827400 nanoseconds     (bandwidth = 33149.44 MB/s)
cache size=1.241615x of data set: 15088000 nanoseconds     (bandwidth = 32576.88 MB/s)
cache size=1.306963x of data set: 13749100 nanoseconds     (bandwidth = 35749.25 MB/s)
cache size=1.375751x of data set: 13245800 nanoseconds     (bandwidth = 37107.61 MB/s)
cache size=1.448158x of data set: 12407100 nanoseconds     (bandwidth = 39616.03 MB/s)
cache size=1.524377x of data set: 11718800 nanoseconds     (bandwidth = 41942.86 MB/s)
cache size=1.604608x of data set: 11337000 nanoseconds     (bandwidth = 43355.39 MB/s)
cache size=1.689061x of data set: 11251200 nanoseconds     (bandwidth = 43686.01 MB/s)
cache size=1.777959x of data set: 10579500 nanoseconds     (bandwidth = 46459.66 MB/s)
cache size=1.871535x of data set: 10150900 nanoseconds     (bandwidth = 48421.32 MB/s)
cache size=1.970037x of data set: 10522400 nanoseconds     (bandwidth = 46711.78 MB/s)
cache size=2.073723x of data set: 9800500 nanoseconds     (bandwidth = 50152.54 MB/s)
cache size=2.182867x of data set: 9499400 nanoseconds     (bandwidth = 51742.22 MB/s)
cache size=2.297755x of data set: 9177000 nanoseconds     (bandwidth = 53559.99 MB/s)
cache size=2.418689x of data set: 9525900 nanoseconds     (bandwidth = 51598.27 MB/s)
cache size=2.545988x of data set: 8923400 nanoseconds     (bandwidth = 55082.14 MB/s)
cache size=2.679988x of data set: 9037300 nanoseconds     (bandwidth = 54387.93 MB/s)
cache size=2.821040x of data set: 8433500 nanoseconds     (bandwidth = 58281.85 MB/s)
cache size=2.969516x of data set: 8390700 nanoseconds     (bandwidth = 58579.14 MB/s)
cache size=3.125806x of data set: 8337800 nanoseconds     (bandwidth = 58950.80 MB/s)
cache size=3.290322x of data set: 9823900 nanoseconds     (bandwidth = 50033.08 MB/s)
cache size=3.463497x of data set: 9015700 nanoseconds     (bandwidth = 54518.23 MB/s)
cache size=3.645786x of data set: 8158500 nanoseconds     (bandwidth = 60246.37 MB/s)
cache size=3.837670x of data set: 8351000 nanoseconds     (bandwidth = 58857.62 MB/s)
cache size=4.039652x of data set: 8040600 nanoseconds     (bandwidth = 61129.77 MB/s)
cache size=4.252265x of data set: 8385800 nanoseconds     (bandwidth = 58613.37 MB/s)
cache size=4.476069x of data set: 8242600 nanoseconds     (bandwidth = 59631.67 MB/s)
cache size=4.711651x of data set: 8217000 nanoseconds     (bandwidth = 59817.45 MB/s)
cache size=4.959633x of data set: 8025500 nanoseconds     (bandwidth = 61244.78 MB/s)
cache size=5.220666x of data set: 8099900 nanoseconds     (bandwidth = 60682.23 MB/s)
cache size=5.495438x of data set: 8085300 nanoseconds     (bandwidth = 60791.81 MB/s)
cache size=5.784672x of data set: 8040800 nanoseconds     (bandwidth = 61128.25 MB/s)
cache size=6.089128x of data set: 8161900 nanoseconds     (bandwidth = 60221.27 MB/s)
cache size=6.409609x of data set: 8014100 nanoseconds     (bandwidth = 61331.90 MB/s)
cache size=6.746957x of data set: 7987800 nanoseconds     (bandwidth = 61533.84 MB/s)
cache size=7.102060x of data set: 7949000 nanoseconds     (bandwidth = 61834.19 MB/s)
cache size=7.475852x of data set: 8567200 nanoseconds     (bandwidth = 57372.30 MB/s)
cache size=7.869318x of data set: 8031100 nanoseconds     (bandwidth = 61202.08 MB/s)
cache size=8.283493x of data set: 8036200 nanoseconds     (bandwidth = 61163.24 MB/s)
cache size=8.719466x of data set: 7897600 nanoseconds     (bandwidth = 62236.63 MB/s)
cache size=9.178385x of data set: 7927700 nanoseconds     (bandwidth = 62000.33 MB/s)
cache size=9.661458x of data set: 7937300 nanoseconds     (bandwidth = 61925.34 MB/s)
cache size=10.169956x of data set: 8063100 nanoseconds     (bandwidth = 60959.18 MB/s)
cache size=10.705217x of data set: 7895500 nanoseconds     (bandwidth = 62253.18 MB/s)
cache size=11.268649x of data set: 8213600 nanoseconds     (bandwidth = 59842.21 MB/s)
cache size=11.861736x of data set: 7749700 nanoseconds     (bandwidth = 63424.39 MB/s)
cache size=12.486038x of data set: 7862900 nanoseconds     (bandwidth = 62511.29 MB/s)
cache size=13.143198x of data set: 7681900 nanoseconds     (bandwidth = 63984.17 MB/s)
cache size=13.834945x of data set: 7784700 nanoseconds     (bandwidth = 63139.23 MB/s)
cache size=14.563100x of data set: 7681000 nanoseconds     (bandwidth = 63991.67 MB/s)
cache size=15.329579x of data set: 7648000 nanoseconds     (bandwidth = 64267.78 MB/s)
cache size=16.136399x of data set: 7640700 nanoseconds     (bandwidth = 64329.18 MB/s)
cache size=16.985683x of data set: 7556000 nanoseconds     (bandwidth = 65050.29 MB/s)
cache size=17.879667x of data set: 7569800 nanoseconds     (bandwidth = 64931.70 MB/s)
cache size=18.820702x of data set: 7480000 nanoseconds     (bandwidth = 65711.23 MB/s)
cache size=19.811265x of data set: 8179500 nanoseconds     (bandwidth = 60091.69 MB/s)
cache size=20.853963x of data set: 7620800 nanoseconds     (bandwidth = 64497.17 MB/s)
cache size=21.951540x of data set: 7458200 nanoseconds     (bandwidth = 65903.30 MB/s)
cache size=23.106884x of data set: 7322800 nanoseconds     (bandwidth = 67121.87 MB/s)
cache size=24.323036x of data set: 7318700 nanoseconds     (bandwidth = 67159.47 MB/s)
cache size=25.603196x of data set: 7206900 nanoseconds     (bandwidth = 68201.31 MB/s)
cache size=26.950733x of data set: 7438500 nanoseconds     (bandwidth = 66077.84 MB/s)
cache size=28.369192x of data set: 7326600 nanoseconds     (bandwidth = 67087.05 MB/s)
cache size=29.862308x of data set: 7171800 nanoseconds     (bandwidth = 68535.10 MB/s)
cache size=31.434008x of data set: 6979400 nanoseconds     (bandwidth = 70424.39 MB/s)
cache size=33.088430x of data set: 6970200 nanoseconds     (bandwidth = 70517.35 MB/s)
cache size=34.829926x of data set: 6913200 nanoseconds     (bandwidth = 71098.77 MB/s)
cache size=36.663080x of data set: 6933700 nanoseconds     (bandwidth = 70888.56 MB/s)
cache size=38.592716x of data set: 6767700 nanoseconds     (bandwidth = 72627.33 MB/s)
cache size=40.623911x of data set: 6760700 nanoseconds     (bandwidth = 72702.53 MB/s)
cache size=42.762012x of data set: 6708700 nanoseconds     (bandwidth = 73266.06 MB/s)
cache size=45.012644x of data set: 6906000 nanoseconds     (bandwidth = 71172.89 MB/s)
cache size=47.381730x of data set: 7237100 nanoseconds     (bandwidth = 67916.71 MB/s)
cache size=49.875506x of data set: 6907400 nanoseconds     (bandwidth = 71158.47 MB/s)
cache size=52.500532x of data set: 6546700 nanoseconds     (bandwidth = 75079.05 MB/s)
cache size=55.263718x of data set: 6480400 nanoseconds     (bandwidth = 75847.17 MB/s)
cache size=58.172335x of data set: 6710400 nanoseconds     (bandwidth = 73247.50 MB/s)
cache size=61.234037x of data set: 6446900 nanoseconds     (bandwidth = 76241.29 MB/s)
cache size=64.456881x of data set: 6552400 nanoseconds     (bandwidth = 75013.74 MB/s)
cache size=67.849348x of data set: 6427500 nanoseconds     (bandwidth = 76471.41 MB/s)
cache size=71.420367x of data set: 6443300 nanoseconds     (bandwidth = 76283.89 MB/s)
cache size=75.179333x of data set: 6379100 nanoseconds     (bandwidth = 77051.62 MB/s)
cache size=79.136140x of data set: 6387300 nanoseconds     (bandwidth = 76952.70 MB/s)
cache size=83.301200x of data set: 6358900 nanoseconds     (bandwidth = 77296.39 MB/s)
cache size=87.685474x of data set: 6347800 nanoseconds     (bandwidth = 77431.55 MB/s)
cache size=92.300499x of data set: 6402200 nanoseconds     (bandwidth = 76773.61 MB/s)
cache size=97.158420x of data set: 6316200 nanoseconds     (bandwidth = 77818.94 MB/s)
cache size=102.272021x of data set: 6330600 nanoseconds     (bandwidth = 77641.93 MB/s)
```
