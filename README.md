# load-store-pass

A simple compiler pass that finds 
for every store in a function 
all the loads (from the same function) 
that the store aliases with, i.e., have the same target address
