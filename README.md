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
24-thread random-access performance benchmark.
Element size = 61440 bytes
Array size = 4608000000 bytes
Cache size = 46080000 bytes
init...
...complete
hit-rate=1.000013%: 122873100 nanoseconds     (bandwidth = 3996.22 MB/s)
hit-rate=1.052646%: 98042100 nanoseconds     (bandwidth = 5008.34 MB/s)
hit-rate=1.108048%: 100437400 nanoseconds     (bandwidth = 4888.90 MB/s)
hit-rate=1.166366%: 97581200 nanoseconds     (bandwidth = 5032.00 MB/s)
hit-rate=1.227754%: 94900900 nanoseconds     (bandwidth = 5174.12 MB/s)
hit-rate=1.292373%: 99335000 nanoseconds     (bandwidth = 4943.16 MB/s)
hit-rate=1.360392%: 96746800 nanoseconds     (bandwidth = 5075.40 MB/s)
hit-rate=1.431992%: 92546500 nanoseconds     (bandwidth = 5305.75 MB/s)
hit-rate=1.507360%: 98821100 nanoseconds     (bandwidth = 4968.86 MB/s)
hit-rate=1.586695%: 98276600 nanoseconds     (bandwidth = 4996.39 MB/s)
hit-rate=1.670205%: 98473700 nanoseconds     (bandwidth = 4986.39 MB/s)
hit-rate=1.758110%: 97101900 nanoseconds     (bandwidth = 5056.84 MB/s)
hit-rate=1.850642%: 97861300 nanoseconds     (bandwidth = 5017.60 MB/s)
hit-rate=1.948045%: 100012700 nanoseconds     (bandwidth = 4909.66 MB/s)
hit-rate=2.050573%: 95574300 nanoseconds     (bandwidth = 5137.66 MB/s)
hit-rate=2.158498%: 97363900 nanoseconds     (bandwidth = 5043.23 MB/s)
hit-rate=2.272103%: 94960900 nanoseconds     (bandwidth = 5170.85 MB/s)
hit-rate=2.391688%: 96869600 nanoseconds     (bandwidth = 5068.96 MB/s)
hit-rate=2.517566%: 95920500 nanoseconds     (bandwidth = 5119.12 MB/s)
hit-rate=2.650070%: 94394000 nanoseconds     (bandwidth = 5201.90 MB/s)
hit-rate=2.789547%: 95283700 nanoseconds     (bandwidth = 5153.33 MB/s)
hit-rate=2.936365%: 94589100 nanoseconds     (bandwidth = 5191.17 MB/s)
hit-rate=3.090911%: 94357300 nanoseconds     (bandwidth = 5203.93 MB/s)
hit-rate=3.253590%: 92841400 nanoseconds     (bandwidth = 5288.90 MB/s)
hit-rate=3.424832%: 93474200 nanoseconds     (bandwidth = 5253.09 MB/s)
hit-rate=3.605086%: 95805800 nanoseconds     (bandwidth = 5125.25 MB/s)
hit-rate=3.794828%: 93223600 nanoseconds     (bandwidth = 5267.21 MB/s)
hit-rate=3.994555%: 90335300 nanoseconds     (bandwidth = 5435.62 MB/s)
hit-rate=4.204795%: 93180300 nanoseconds     (bandwidth = 5269.66 MB/s)
hit-rate=4.426100%: 92470300 nanoseconds     (bandwidth = 5310.12 MB/s)
hit-rate=4.659053%: 91363800 nanoseconds     (bandwidth = 5374.43 MB/s)
hit-rate=4.904266%: 90572600 nanoseconds     (bandwidth = 5421.38 MB/s)
hit-rate=5.162385%: 92017300 nanoseconds     (bandwidth = 5336.26 MB/s)
hit-rate=5.434090%: 92283500 nanoseconds     (bandwidth = 5320.87 MB/s)
hit-rate=5.720095%: 92352600 nanoseconds     (bandwidth = 5316.89 MB/s)
hit-rate=6.021152%: 94189300 nanoseconds     (bandwidth = 5213.21 MB/s)
hit-rate=6.338055%: 91707600 nanoseconds     (bandwidth = 5354.28 MB/s)
hit-rate=6.671637%: 88706500 nanoseconds     (bandwidth = 5535.43 MB/s)
hit-rate=7.022776%: 90947500 nanoseconds     (bandwidth = 5399.03 MB/s)
hit-rate=7.392395%: 88151900 nanoseconds     (bandwidth = 5570.25 MB/s)
hit-rate=7.781469%: 88305400 nanoseconds     (bandwidth = 5560.57 MB/s)
hit-rate=8.191020%: 87837300 nanoseconds     (bandwidth = 5590.20 MB/s)
hit-rate=8.622126%: 87235900 nanoseconds     (bandwidth = 5628.74 MB/s)
hit-rate=9.075922%: 87725800 nanoseconds     (bandwidth = 5597.31 MB/s)
hit-rate=9.553602%: 83939000 nanoseconds     (bandwidth = 5849.83 MB/s)
hit-rate=10.056423%: 84792600 nanoseconds     (bandwidth = 5790.94 MB/s)
hit-rate=10.585709%: 83935400 nanoseconds     (bandwidth = 5850.08 MB/s)
hit-rate=11.142851%: 82292400 nanoseconds     (bandwidth = 5966.88 MB/s)
hit-rate=11.729317%: 79746400 nanoseconds     (bandwidth = 6157.37 MB/s)
hit-rate=12.346650%: 79019000 nanoseconds     (bandwidth = 6214.06 MB/s)
hit-rate=12.996474%: 80824100 nanoseconds     (bandwidth = 6075.27 MB/s)
hit-rate=13.680498%: 75851800 nanoseconds     (bandwidth = 6473.52 MB/s)
hit-rate=14.400525%: 79280100 nanoseconds     (bandwidth = 6193.59 MB/s)
hit-rate=15.158447%: 76853000 nanoseconds     (bandwidth = 6389.19 MB/s)
hit-rate=15.956260%: 76837600 nanoseconds     (bandwidth = 6390.47 MB/s)
hit-rate=16.796063%: 73090700 nanoseconds     (bandwidth = 6718.07 MB/s)
hit-rate=17.680067%: 74140000 nanoseconds     (bandwidth = 6622.99 MB/s)
hit-rate=18.610596%: 73274600 nanoseconds     (bandwidth = 6701.21 MB/s)
hit-rate=19.590101%: 69784400 nanoseconds     (bandwidth = 7036.36 MB/s)
hit-rate=20.621159%: 70515200 nanoseconds     (bandwidth = 6963.44 MB/s)
hit-rate=21.706484%: 69153200 nanoseconds     (bandwidth = 7100.59 MB/s)
hit-rate=22.848930%: 67830900 nanoseconds     (bandwidth = 7239.01 MB/s)
hit-rate=24.051505%: 65870300 nanoseconds     (bandwidth = 7454.47 MB/s)
hit-rate=25.317374%: 62998600 nanoseconds     (bandwidth = 7794.28 MB/s)
hit-rate=26.649867%: 60301400 nanoseconds     (bandwidth = 8142.90 MB/s)
hit-rate=28.052492%: 61136300 nanoseconds     (bandwidth = 8031.70 MB/s)
hit-rate=29.528939%: 57003100 nanoseconds     (bandwidth = 8614.07 MB/s)
hit-rate=31.083094%: 56163000 nanoseconds     (bandwidth = 8742.92 MB/s)
hit-rate=32.719046%: 52368500 nanoseconds     (bandwidth = 9376.41 MB/s)
hit-rate=34.441101%: 49755300 nanoseconds     (bandwidth = 9868.87 MB/s)
hit-rate=36.253790%: 48756800 nanoseconds     (bandwidth = 10070.97 MB/s)
hit-rate=38.161885%: 45466800 nanoseconds     (bandwidth = 10799.71 MB/s)
hit-rate=40.170405%: 42651400 nanoseconds     (bandwidth = 11512.60 MB/s)
hit-rate=42.284637%: 40478700 nanoseconds     (bandwidth = 12130.54 MB/s)
hit-rate=44.510144%: 38251300 nanoseconds     (bandwidth = 12836.91 MB/s)
hit-rate=46.852783%: 33694200 nanoseconds     (bandwidth = 14573.09 MB/s)
hit-rate=49.318719%: 31098700 nanoseconds     (bandwidth = 15789.36 MB/s)
hit-rate=51.914441%: 27132500 nanoseconds     (bandwidth = 18097.43 MB/s)
hit-rate=54.646780%: 23116800 nanoseconds     (bandwidth = 21241.20 MB/s)
hit-rate=57.522926%: 18155200 nanoseconds     (bandwidth = 27046.16 MB/s)
hit-rate=60.550449%: 14683700 nanoseconds     (bandwidth = 33440.38 MB/s)
hit-rate=63.737315%: 10879000 nanoseconds     (bandwidth = 45135.44 MB/s)
hit-rate=67.091910%: 7646800 nanoseconds     (bandwidth = 64213.59 MB/s)
hit-rate=70.623063%: 6386300 nanoseconds     (bandwidth = 76887.79 MB/s)
hit-rate=74.340067%: 6229000 nanoseconds     (bandwidth = 78829.42 MB/s)
hit-rate=78.252702%: 5779300 nanoseconds     (bandwidth = 84963.31 MB/s)
hit-rate=82.371265%: 5289700 nanoseconds     (bandwidth = 92827.28 MB/s)
hit-rate=86.706595%: 5902000 nanoseconds     (bandwidth = 83196.96 MB/s)
hit-rate=91.270100%: 5901500 nanoseconds     (bandwidth = 83204.01 MB/s)
hit-rate=96.073789%: 5817000 nanoseconds     (bandwidth = 84412.67 MB/s)
cache size=1.011303x of data set: 5512600 nanoseconds     (bandwidth = 89073.85 MB/s)
cache size=1.064530x of data set: 5052400 nanoseconds     (bandwidth = 97187.17 MB/s)
cache size=1.120557x of data set: 3954400 nanoseconds     (bandwidth = 124172.69 MB/s)
cache size=1.179534x of data set: 4328500 nanoseconds     (bandwidth = 113440.79 MB/s)
cache size=1.241615x of data set: 4014800 nanoseconds     (bandwidth = 122304.59 MB/s)
cache size=1.306963x of data set: 3455300 nanoseconds     (bandwidth = 142108.78 MB/s)
cache size=1.375751x of data set: 3333500 nanoseconds     (bandwidth = 147301.18 MB/s)
cache size=1.448158x of data set: 3513500 nanoseconds     (bandwidth = 139754.80 MB/s)
cache size=1.524377x of data set: 3324400 nanoseconds     (bandwidth = 147704.39 MB/s)
cache size=1.604608x of data set: 2982200 nanoseconds     (bandwidth = 164653.10 MB/s)
cache size=1.689061x of data set: 2969700 nanoseconds     (bandwidth = 165346.16 MB/s)
cache size=1.777959x of data set: 2765900 nanoseconds     (bandwidth = 177529.37 MB/s)
cache size=1.871535x of data set: 2826000 nanoseconds     (bandwidth = 173753.89 MB/s)
cache size=1.970037x of data set: 2595200 nanoseconds     (bandwidth = 189206.41 MB/s)
cache size=2.073723x of data set: 2701000 nanoseconds     (bandwidth = 181795.07 MB/s)
cache size=2.182867x of data set: 2785400 nanoseconds     (bandwidth = 176286.52 MB/s)
cache size=2.297755x of data set: 2524000 nanoseconds     (bandwidth = 194543.77 MB/s)
cache size=2.418689x of data set: 2497500 nanoseconds     (bandwidth = 196608.00 MB/s)
cache size=2.545988x of data set: 2413000 nanoseconds     (bandwidth = 203492.95 MB/s)
cache size=2.679988x of data set: 2332700 nanoseconds     (bandwidth = 210497.91 MB/s)
cache size=2.821040x of data set: 2275300 nanoseconds     (bandwidth = 215808.24 MB/s)
cache size=2.969516x of data set: 2378800 nanoseconds     (bandwidth = 206418.56 MB/s)
cache size=3.125806x of data set: 2387700 nanoseconds     (bandwidth = 205649.15 MB/s)
cache size=3.290322x of data set: 2464800 nanoseconds     (bandwidth = 199216.36 MB/s)
cache size=3.463497x of data set: 2366800 nanoseconds     (bandwidth = 207465.13 MB/s)
cache size=3.645786x of data set: 2678800 nanoseconds     (bandwidth = 183301.66 MB/s)
cache size=3.837670x of data set: 2373100 nanoseconds     (bandwidth = 206914.37 MB/s)
cache size=4.039652x of data set: 2452300 nanoseconds     (bandwidth = 200231.82 MB/s)
cache size=4.252265x of data set: 2408100 nanoseconds     (bandwidth = 203907.01 MB/s)
cache size=4.476069x of data set: 2329100 nanoseconds     (bandwidth = 210823.27 MB/s)
cache size=4.711651x of data set: 2492600 nanoseconds     (bandwidth = 196994.50 MB/s)
cache size=4.959633x of data set: 2587300 nanoseconds     (bandwidth = 189784.13 MB/s)
cache size=5.220666x of data set: 2886200 nanoseconds     (bandwidth = 170129.75 MB/s)
cache size=5.495438x of data set: 2695400 nanoseconds     (bandwidth = 182172.77 MB/s)
cache size=5.784672x of data set: 2313600 nanoseconds     (bandwidth = 212235.68 MB/s)
cache size=6.089128x of data set: 2645100 nanoseconds     (bandwidth = 185637.02 MB/s)
cache size=6.409609x of data set: 2614100 nanoseconds     (bandwidth = 187838.45 MB/s)
cache size=6.746957x of data set: 2369200 nanoseconds     (bandwidth = 207254.97 MB/s)
cache size=7.102060x of data set: 2573600 nanoseconds     (bandwidth = 190794.40 MB/s)
cache size=7.475852x of data set: 2368700 nanoseconds     (bandwidth = 207298.72 MB/s)
cache size=7.869318x of data set: 2399600 nanoseconds     (bandwidth = 204629.30 MB/s)
cache size=8.283493x of data set: 2326000 nanoseconds     (bandwidth = 211104.25 MB/s)
cache size=8.719466x of data set: 2549400 nanoseconds     (bandwidth = 192605.51 MB/s)
cache size=9.178385x of data set: 2442900 nanoseconds     (bandwidth = 201002.28 MB/s)
cache size=9.661458x of data set: 2376500 nanoseconds     (bandwidth = 206618.34 MB/s)
cache size=10.169956x of data set: 2370000 nanoseconds     (bandwidth = 207185.01 MB/s)
cache size=10.705217x of data set: 2550300 nanoseconds     (bandwidth = 192537.54 MB/s)
cache size=11.268649x of data set: 2307000 nanoseconds     (bandwidth = 212842.86 MB/s)
cache size=11.861736x of data set: 2566500 nanoseconds     (bandwidth = 191322.22 MB/s)
cache size=12.486038x of data set: 2530400 nanoseconds     (bandwidth = 194051.72 MB/s)
cache size=13.143198x of data set: 2412600 nanoseconds     (bandwidth = 203526.68 MB/s)
cache size=13.834945x of data set: 2585900 nanoseconds     (bandwidth = 189886.88 MB/s)
cache size=14.563100x of data set: 2838600 nanoseconds     (bandwidth = 172982.63 MB/s)
cache size=15.329579x of data set: 2493700 nanoseconds     (bandwidth = 196907.60 MB/s)
cache size=16.136399x of data set: 2459500 nanoseconds     (bandwidth = 199645.65 MB/s)
cache size=16.985683x of data set: 2361700 nanoseconds     (bandwidth = 207913.15 MB/s)
cache size=17.879667x of data set: 2463500 nanoseconds     (bandwidth = 199321.49 MB/s)
cache size=18.820702x of data set: 2780400 nanoseconds     (bandwidth = 176603.54 MB/s)
cache size=19.811265x of data set: 2437700 nanoseconds     (bandwidth = 201431.05 MB/s)
cache size=20.853963x of data set: 2566900 nanoseconds     (bandwidth = 191292.41 MB/s)
cache size=21.951540x of data set: 2853800 nanoseconds     (bandwidth = 172061.28 MB/s)
cache size=23.106884x of data set: 2796100 nanoseconds     (bandwidth = 175611.92 MB/s)
cache size=24.323036x of data set: 2685200 nanoseconds     (bandwidth = 182864.77 MB/s)
cache size=25.603196x of data set: 2906200 nanoseconds     (bandwidth = 168958.94 MB/s)
cache size=26.950733x of data set: 2870500 nanoseconds     (bandwidth = 171060.26 MB/s)
cache size=28.369192x of data set: 3080500 nanoseconds     (bandwidth = 159398.95 MB/s)
cache size=29.862308x of data set: 3118500 nanoseconds     (bandwidth = 157456.62 MB/s)
cache size=31.434008x of data set: 2706400 nanoseconds     (bandwidth = 181432.34 MB/s)
cache size=33.088430x of data set: 2291300 nanoseconds     (bandwidth = 214301.26 MB/s)
cache size=34.829926x of data set: 2452300 nanoseconds     (bandwidth = 200231.82 MB/s)
cache size=36.663080x of data set: 2470400 nanoseconds     (bandwidth = 198764.77 MB/s)
cache size=38.592716x of data set: 2735100 nanoseconds     (bandwidth = 179528.53 MB/s)
cache size=40.623911x of data set: 2498700 nanoseconds     (bandwidth = 196513.58 MB/s)
cache size=42.762012x of data set: 2652900 nanoseconds     (bandwidth = 185091.21 MB/s)
cache size=45.012644x of data set: 2728100 nanoseconds     (bandwidth = 179989.18 MB/s)
cache size=47.381730x of data set: 2789800 nanoseconds     (bandwidth = 176008.49 MB/s)
cache size=49.875506x of data set: 2978100 nanoseconds     (bandwidth = 164879.78 MB/s)
cache size=52.500532x of data set: 2830500 nanoseconds     (bandwidth = 173477.65 MB/s)
cache size=55.263718x of data set: 3047800 nanoseconds     (bandwidth = 161109.15 MB/s)
cache size=58.172335x of data set: 3428700 nanoseconds     (bandwidth = 143211.27 MB/s)
cache size=61.234037x of data set: 3438700 nanoseconds     (bandwidth = 142794.80 MB/s)
cache size=64.456881x of data set: 3743700 nanoseconds     (bandwidth = 131161.28 MB/s)
cache size=67.849348x of data set: 3492400 nanoseconds     (bandwidth = 140599.15 MB/s)
cache size=71.420367x of data set: 4049400 nanoseconds     (bandwidth = 121259.56 MB/s)
cache size=75.179333x of data set: 4521700 nanoseconds     (bandwidth = 108593.78 MB/s)
cache size=79.136140x of data set: 3880000 nanoseconds     (bandwidth = 126553.73 MB/s)
cache size=83.301200x of data set: 3838200 nanoseconds     (bandwidth = 127931.97 MB/s)
cache size=87.685474x of data set: 3966900 nanoseconds     (bandwidth = 123781.41 MB/s)
cache size=92.300499x of data set: 4389200 nanoseconds     (bandwidth = 111871.98 MB/s)
cache size=97.158420x of data set: 4251800 nanoseconds     (bandwidth = 115487.20 MB/s)
cache size=102.272021x of data set: 4519700 nanoseconds     (bandwidth = 108641.83 MB/s)
```
