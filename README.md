# TrackMaster

A C++ scheduling engine that resolves track lane limits, event overlaps, and athlete registration conflicts during sports meets.

## Features
* **Maximized Lane Throughput:** Pack the most races possible onto a single lane without overlaps using greedy interval sorting.
* **Clash-Free Timetables:** Model event brackets as a graph and use vertex-coloring to ensure multi-discipline athletes are never double-booked.
* **Safe Registration Pipeline:** Process real-time changes using a strict data-sanitizing sequential queue.
* **High-Speed Execution:** Built on pure STL containers to deliver optimized schedules in under 150ms.
