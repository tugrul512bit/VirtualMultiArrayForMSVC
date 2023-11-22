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
Array size = 14745600000 bytes
Cache size = 147456000 bytes
init...
...complete
hit-rate=1.000004%: 154254100 nanoseconds     (bandwidth = 3183.24 MB/s)
hit-rate=1.052636%: 106855200 nanoseconds     (bandwidth = 4595.27 MB/s)
hit-rate=1.108038%: 108114200 nanoseconds     (bandwidth = 4541.76 MB/s)
hit-rate=1.166356%: 107224400 nanoseconds     (bandwidth = 4579.45 MB/s)
hit-rate=1.227743%: 107678500 nanoseconds     (bandwidth = 4560.13 MB/s)
hit-rate=1.292361%: 108068300 nanoseconds     (bandwidth = 4543.69 MB/s)
hit-rate=1.360380%: 107913400 nanoseconds     (bandwidth = 4550.21 MB/s)
hit-rate=1.431979%: 105793400 nanoseconds     (bandwidth = 4641.39 MB/s)
hit-rate=1.507346%: 104050500 nanoseconds     (bandwidth = 4719.14 MB/s)
hit-rate=1.586680%: 108605700 nanoseconds     (bandwidth = 4521.20 MB/s)
hit-rate=1.670190%: 110305100 nanoseconds     (bandwidth = 4451.55 MB/s)
hit-rate=1.758094%: 106613800 nanoseconds     (bandwidth = 4605.67 MB/s)
hit-rate=1.850626%: 105610900 nanoseconds     (bandwidth = 4649.41 MB/s)
hit-rate=1.948027%: 104538300 nanoseconds     (bandwidth = 4697.12 MB/s)
hit-rate=2.050555%: 106502800 nanoseconds     (bandwidth = 4610.47 MB/s)
hit-rate=2.158479%: 110040500 nanoseconds     (bandwidth = 4462.25 MB/s)
hit-rate=2.272083%: 105654500 nanoseconds     (bandwidth = 4647.49 MB/s)
hit-rate=2.391666%: 105532400 nanoseconds     (bandwidth = 4652.87 MB/s)
hit-rate=2.517543%: 107363600 nanoseconds     (bandwidth = 4573.51 MB/s)
hit-rate=2.650045%: 104178200 nanoseconds     (bandwidth = 4713.35 MB/s)
hit-rate=2.789521%: 106508000 nanoseconds     (bandwidth = 4610.25 MB/s)
hit-rate=2.936338%: 100311700 nanoseconds     (bandwidth = 4895.03 MB/s)
hit-rate=3.090882%: 101152600 nanoseconds     (bandwidth = 4854.33 MB/s)
hit-rate=3.253561%: 105881500 nanoseconds     (bandwidth = 4637.53 MB/s)
hit-rate=3.424801%: 104174900 nanoseconds     (bandwidth = 4713.50 MB/s)
hit-rate=3.605053%: 105818300 nanoseconds     (bandwidth = 4640.30 MB/s)
hit-rate=3.794793%: 104207500 nanoseconds     (bandwidth = 4712.03 MB/s)
hit-rate=3.994519%: 101466800 nanoseconds     (bandwidth = 4839.30 MB/s)
hit-rate=4.204757%: 101308500 nanoseconds     (bandwidth = 4846.86 MB/s)
hit-rate=4.426060%: 104461300 nanoseconds     (bandwidth = 4700.58 MB/s)
hit-rate=4.659010%: 106429200 nanoseconds     (bandwidth = 4613.66 MB/s)
hit-rate=4.904221%: 104874700 nanoseconds     (bandwidth = 4682.05 MB/s)
hit-rate=5.162338%: 102845200 nanoseconds     (bandwidth = 4774.44 MB/s)
hit-rate=5.434040%: 97161700 nanoseconds     (bandwidth = 5053.72 MB/s)
hit-rate=5.720042%: 100875400 nanoseconds     (bandwidth = 4867.67 MB/s)
hit-rate=6.021097%: 101527100 nanoseconds     (bandwidth = 4836.43 MB/s)
hit-rate=6.337997%: 99111700 nanoseconds     (bandwidth = 4954.29 MB/s)
hit-rate=6.671576%: 99509700 nanoseconds     (bandwidth = 4934.48 MB/s)
hit-rate=7.022711%: 99023600 nanoseconds     (bandwidth = 4958.70 MB/s)
hit-rate=7.392328%: 98794100 nanoseconds     (bandwidth = 4970.22 MB/s)
hit-rate=7.781397%: 100177400 nanoseconds     (bandwidth = 4901.59 MB/s)
hit-rate=8.190945%: 100143500 nanoseconds     (bandwidth = 4903.25 MB/s)
hit-rate=8.622047%: 102027900 nanoseconds     (bandwidth = 4812.69 MB/s)
hit-rate=9.075839%: 100965100 nanoseconds     (bandwidth = 4863.35 MB/s)
hit-rate=9.553515%: 96457000 nanoseconds     (bandwidth = 5090.65 MB/s)
hit-rate=10.056331%: 97295400 nanoseconds     (bandwidth = 5046.78 MB/s)
hit-rate=10.585612%: 97625700 nanoseconds     (bandwidth = 5029.71 MB/s)
hit-rate=11.142749%: 95366500 nanoseconds     (bandwidth = 5148.86 MB/s)
hit-rate=11.729210%: 98304800 nanoseconds     (bandwidth = 4994.96 MB/s)
hit-rate=12.346537%: 95195500 nanoseconds     (bandwidth = 5158.11 MB/s)
hit-rate=12.996354%: 93306900 nanoseconds     (bandwidth = 5262.51 MB/s)
hit-rate=13.680373%: 94686800 nanoseconds     (bandwidth = 5185.82 MB/s)
hit-rate=14.400393%: 92787400 nanoseconds     (bandwidth = 5291.97 MB/s)
hit-rate=15.158308%: 89229400 nanoseconds     (bandwidth = 5502.99 MB/s)
hit-rate=15.956114%: 92563300 nanoseconds     (bandwidth = 5304.79 MB/s)
hit-rate=16.795909%: 89347100 nanoseconds     (bandwidth = 5495.74 MB/s)
hit-rate=17.679904%: 88680200 nanoseconds     (bandwidth = 5537.07 MB/s)
hit-rate=18.610426%: 89641000 nanoseconds     (bandwidth = 5477.72 MB/s)
hit-rate=19.589922%: 88710700 nanoseconds     (bandwidth = 5535.17 MB/s)
hit-rate=20.620970%: 86422400 nanoseconds     (bandwidth = 5681.73 MB/s)
hit-rate=21.706285%: 83842200 nanoseconds     (bandwidth = 5856.58 MB/s)
hit-rate=22.848721%: 79548600 nanoseconds     (bandwidth = 6172.69 MB/s)
hit-rate=24.051285%: 82466300 nanoseconds     (bandwidth = 5954.29 MB/s)
hit-rate=25.317142%: 82502400 nanoseconds     (bandwidth = 5951.69 MB/s)
hit-rate=26.649623%: 79244000 nanoseconds     (bandwidth = 6196.41 MB/s)
hit-rate=28.052235%: 77038400 nanoseconds     (bandwidth = 6373.81 MB/s)
hit-rate=29.528668%: 75099800 nanoseconds     (bandwidth = 6538.35 MB/s)
hit-rate=31.082809%: 73232200 nanoseconds     (bandwidth = 6705.09 MB/s)
hit-rate=32.718746%: 74105200 nanoseconds     (bandwidth = 6626.10 MB/s)
hit-rate=34.440785%: 71989000 nanoseconds     (bandwidth = 6820.88 MB/s)
hit-rate=36.253458%: 67746700 nanoseconds     (bandwidth = 7248.01 MB/s)
hit-rate=38.161535%: 67227200 nanoseconds     (bandwidth = 7304.02 MB/s)
hit-rate=40.170037%: 64385700 nanoseconds     (bandwidth = 7626.36 MB/s)
hit-rate=42.284249%: 61251900 nanoseconds     (bandwidth = 8016.54 MB/s)
hit-rate=44.509736%: 62133700 nanoseconds     (bandwidth = 7902.77 MB/s)
hit-rate=46.852354%: 55840400 nanoseconds     (bandwidth = 8793.43 MB/s)
hit-rate=49.318267%: 54168300 nanoseconds     (bandwidth = 9064.87 MB/s)
hit-rate=51.913965%: 52502100 nanoseconds     (bandwidth = 9352.55 MB/s)
hit-rate=54.646279%: 48852400 nanoseconds     (bandwidth = 10051.27 MB/s)
hit-rate=57.522399%: 44527900 nanoseconds     (bandwidth = 11027.43 MB/s)
hit-rate=60.549894%: 42039500 nanoseconds     (bandwidth = 11680.17 MB/s)
hit-rate=63.736730%: 37978500 nanoseconds     (bandwidth = 12929.12 MB/s)
hit-rate=67.091295%: 36206300 nanoseconds     (bandwidth = 13561.96 MB/s)
hit-rate=70.622416%: 31281600 nanoseconds     (bandwidth = 15697.04 MB/s)
hit-rate=74.339385%: 26707100 nanoseconds     (bandwidth = 18385.69 MB/s)
hit-rate=78.251984%: 24128400 nanoseconds     (bandwidth = 20350.64 MB/s)
hit-rate=82.370510%: 19296800 nanoseconds     (bandwidth = 25446.11 MB/s)
hit-rate=86.705800%: 14566000 nanoseconds     (bandwidth = 33710.59 MB/s)
hit-rate=91.269263%: 11481300 nanoseconds     (bandwidth = 42767.67 MB/s)
hit-rate=96.072909%: 8680100 nanoseconds     (bandwidth = 56569.45 MB/s)
cache size=1.011294x of data set: 8107400 nanoseconds     (bandwidth = 60565.47 MB/s)
cache size=1.064520x of data set: 7759800 nanoseconds     (bandwidth = 63278.50 MB/s)
cache size=1.120547x of data set: 8074600 nanoseconds     (bandwidth = 60811.49 MB/s)
cache size=1.179523x of data set: 7397200 nanoseconds     (bandwidth = 66380.32 MB/s)
cache size=1.241603x of data set: 7447900 nanoseconds     (bandwidth = 65928.45 MB/s)
cache size=1.306951x of data set: 7198000 nanoseconds     (bandwidth = 68217.35 MB/s)
cache size=1.375738x of data set: 7565900 nanoseconds     (bandwidth = 64900.21 MB/s)
cache size=1.448145x of data set: 6901100 nanoseconds     (bandwidth = 71152.20 MB/s)
cache size=1.524363x of data set: 7240100 nanoseconds     (bandwidth = 67820.68 MB/s)
cache size=1.604593x of data set: 6547100 nanoseconds     (bandwidth = 74999.39 MB/s)
cache size=1.689045x of data set: 6537600 nanoseconds     (bandwidth = 75108.37 MB/s)
cache size=1.777942x of data set: 6837400 nanoseconds     (bandwidth = 71815.09 MB/s)
cache size=1.871518x of data set: 6711000 nanoseconds     (bandwidth = 73167.71 MB/s)
cache size=1.970019x of data set: 6383900 nanoseconds     (bandwidth = 76916.69 MB/s)
cache size=2.073704x of data set: 5664200 nanoseconds     (bandwidth = 86689.82 MB/s)
cache size=2.182847x of data set: 5533600 nanoseconds     (bandwidth = 88735.81 MB/s)
cache size=2.297733x of data set: 5680900 nanoseconds     (bandwidth = 86434.98 MB/s)
cache size=2.418667x of data set: 5134100 nanoseconds     (bandwidth = 95640.61 MB/s)
cache size=2.545965x of data set: 5363000 nanoseconds     (bandwidth = 91558.55 MB/s)
cache size=2.679963x of data set: 4898000 nanoseconds     (bandwidth = 100250.81 MB/s)
cache size=2.821014x of data set: 4448200 nanoseconds     (bandwidth = 110388.13 MB/s)
cache size=2.969488x of data set: 4612400 nanoseconds     (bandwidth = 106458.35 MB/s)
cache size=3.125777x of data set: 4224800 nanoseconds     (bandwidth = 116225.26 MB/s)
cache size=3.290292x of data set: 3559000 nanoseconds     (bandwidth = 137968.10 MB/s)
cache size=3.463465x of data set: 3478500 nanoseconds     (bandwidth = 141160.98 MB/s)
cache size=3.645753x of data set: 3707000 nanoseconds     (bandwidth = 132459.80 MB/s)
cache size=3.837634x of data set: 2940100 nanoseconds     (bandwidth = 167010.81 MB/s)
cache size=4.039615x of data set: 2606000 nanoseconds     (bandwidth = 188422.29 MB/s)
cache size=4.252226x of data set: 2377000 nanoseconds     (bandwidth = 206574.88 MB/s)
cache size=4.476028x of data set: 2353000 nanoseconds     (bandwidth = 208681.89 MB/s)
cache size=4.711608x of data set: 2655500 nanoseconds     (bandwidth = 184909.99 MB/s)
cache size=4.959588x of data set: 2114500 nanoseconds     (bandwidth = 232219.66 MB/s)
cache size=5.220619x of data set: 1809000 nanoseconds     (bandwidth = 271436.42 MB/s)
cache size=5.495388x of data set: 1651100 nanoseconds     (bandwidth = 297394.76 MB/s)
cache size=5.784619x of data set: 1492100 nanoseconds     (bandwidth = 329085.50 MB/s)
cache size=6.089073x of data set: 1529600 nanoseconds     (bandwidth = 321017.57 MB/s)
cache size=6.409550x of data set: 1480000 nanoseconds     (bandwidth = 331776.00 MB/s)
cache size=6.746895x of data set: 1351600 nanoseconds     (bandwidth = 363294.23 MB/s)
cache size=7.101995x of data set: 1764500 nanoseconds     (bandwidth = 278281.94 MB/s)
cache size=7.475784x of data set: 1350000 nanoseconds     (bandwidth = 363724.80 MB/s)
cache size=7.869246x of data set: 1270900 nanoseconds     (bandwidth = 386362.80 MB/s)
cache size=8.283417x of data set: 1417300 nanoseconds     (bandwidth = 346453.45 MB/s)
cache size=8.719386x of data set: 1527200 nanoseconds     (bandwidth = 321522.05 MB/s)
cache size=9.178301x of data set: 1289500 nanoseconds     (bandwidth = 380789.83 MB/s)
cache size=9.661370x of data set: 1339100 nanoseconds     (bandwidth = 366685.45 MB/s)
cache size=10.169863x of data set: 1475300 nanoseconds     (bandwidth = 332832.97 MB/s)
cache size=10.705119x of data set: 1481900 nanoseconds     (bandwidth = 331350.62 MB/s)
cache size=11.268546x of data set: 1371300 nanoseconds     (bandwidth = 358075.17 MB/s)
cache size=11.861627x of data set: 1540000 nanoseconds     (bandwidth = 318849.66 MB/s)
cache size=12.485924x of data set: 1446200 nanoseconds     (bandwidth = 339530.13 MB/s)
cache size=13.143078x of data set: 1523700 nanoseconds     (bandwidth = 322260.60 MB/s)
cache size=13.834818x of data set: 1595000 nanoseconds     (bandwidth = 307854.85 MB/s)
cache size=14.562967x of data set: 1445800 nanoseconds     (bandwidth = 339624.07 MB/s)
cache size=15.329439x of data set: 1341900 nanoseconds     (bandwidth = 365920.32 MB/s)
cache size=16.136251x of data set: 1334600 nanoseconds     (bandwidth = 367921.83 MB/s)
cache size=16.985528x of data set: 1503400 nanoseconds     (bandwidth = 326612.00 MB/s)
cache size=17.879503x of data set: 1415000 nanoseconds     (bandwidth = 347016.59 MB/s)
cache size=18.820529x of data set: 1504300 nanoseconds     (bandwidth = 326416.59 MB/s)
cache size=19.811083x of data set: 1591300 nanoseconds     (bandwidth = 308570.65 MB/s)
cache size=20.853772x of data set: 1563600 nanoseconds     (bandwidth = 314037.15 MB/s)
cache size=21.951339x of data set: 1648000 nanoseconds     (bandwidth = 297954.17 MB/s)
cache size=23.106673x of data set: 1459200 nanoseconds     (bandwidth = 336505.26 MB/s)
cache size=24.322813x of data set: 1382800 nanoseconds     (bandwidth = 355097.25 MB/s)
cache size=25.602961x of data set: 1436500 nanoseconds     (bandwidth = 341822.82 MB/s)
cache size=26.950486x of data set: 1431000 nanoseconds     (bandwidth = 343136.60 MB/s)
cache size=28.368932x of data set: 1494900 nanoseconds     (bandwidth = 328469.11 MB/s)
cache size=29.862034x of data set: 1492600 nanoseconds     (bandwidth = 328975.26 MB/s)
cache size=31.433720x of data set: 1643900 nanoseconds     (bandwidth = 298697.29 MB/s)
cache size=33.088126x of data set: 1853400 nanoseconds     (bandwidth = 264933.89 MB/s)
cache size=34.829607x of data set: 1804200 nanoseconds     (bandwidth = 272158.56 MB/s)
cache size=36.662744x of data set: 1943500 nanoseconds     (bandwidth = 252651.65 MB/s)
cache size=38.592362x of data set: 1789000 nanoseconds     (bandwidth = 274470.92 MB/s)
cache size=40.623539x of data set: 1969400 nanoseconds     (bandwidth = 249328.97 MB/s)
cache size=42.761620x of data set: 2004000 nanoseconds     (bandwidth = 245024.19 MB/s)
cache size=45.012231x of data set: 1481500 nanoseconds     (bandwidth = 331440.08 MB/s)
cache size=47.381296x of data set: 1476700 nanoseconds     (bandwidth = 332517.42 MB/s)
cache size=49.875049x of data set: 1360400 nanoseconds     (bandwidth = 360944.19 MB/s)
cache size=52.500051x of data set: 1480600 nanoseconds     (bandwidth = 331641.55 MB/s)
cache size=55.263212x of data set: 1837700 nanoseconds     (bandwidth = 267197.30 MB/s)
cache size=58.171802x of data set: 1641500 nanoseconds     (bandwidth = 299134.01 MB/s)
cache size=61.233476x of data set: 1692000 nanoseconds     (bandwidth = 290205.96 MB/s)
cache size=64.456290x of data set: 1720900 nanoseconds     (bandwidth = 285332.37 MB/s)
cache size=67.848726x of data set: 1639100 nanoseconds     (bandwidth = 299572.01 MB/s)
cache size=71.419712x of data set: 1814100 nanoseconds     (bandwidth = 270673.33 MB/s)
cache size=75.178644x of data set: 2110100 nanoseconds     (bandwidth = 232703.89 MB/s)
cache size=79.135415x of data set: 1994300 nanoseconds     (bandwidth = 246215.96 MB/s)
cache size=83.300437x of data set: 2066000 nanoseconds     (bandwidth = 237671.09 MB/s)
cache size=87.684670x of data set: 2376300 nanoseconds     (bandwidth = 206635.73 MB/s)
cache size=92.299653x of data set: 2183800 nanoseconds     (bandwidth = 224850.48 MB/s)
cache size=97.157529x of data set: 2243900 nanoseconds     (bandwidth = 218828.15 MB/s)
```
