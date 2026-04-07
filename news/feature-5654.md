`parallelize()`: Added `batch-size()` option

`batch-size()` defines how many consecutive messages each input thread assigns to a single `parallelize()` worker.\
This preserves ordering for those messages on the output side and can also improve the performance of `parallelize()`.
