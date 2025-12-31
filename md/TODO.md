# TODO: WMS-Cpp Codebase Improvements

- [ ] improve and fix receipt system     --> TOP PIRIOTY
- [ ] fix and improve queue system       --> SECOND PIRIOTY
- [ ] detect faults in cli and output for future error handling
- [ ] add smth useful to make the system better 
- [ ] Turing it into real app 
- [ ] Applying fixes from the report 
        -main file is [done]
        -cli.cpp is [done]
        -item files [next]


## Step 1: Fix Errors
- [x] Fix typo "intializeStorage" to "initializeStorage" in Storage.h and Storage.cpp
- [x] Fix "ERROR [404]" to "WARNING" in printWarning function in output.cpp

## Step 2: Add Comments to Core Files
- [x] Add detailed comments to Storage.h and Storage.cpp
- [x] Add detailed comments to output.h and output.cpp
- [x] Add detailed comments to WmsControllers.h and WmsControllers.cpp (including queue methods)
- [x] Add detailed comments to Inventory.h and Inventory.cpp
- [x] Add detailed comments to Item.h and Item.cpp
- [x] Add detailed comments to Receipt.h and Receipt.cpp
- [x] Add detailed comments to cli.h and cli.cpp
- [x] Add detailed comments to main.cpp

## Step 4: Followup
- [x] Compile and test the code
- [ ] Verify queue functionality and CLI commands
