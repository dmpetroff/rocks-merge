# rocks-merge
Tiny program to demonstrate suspicious merge/compaction interactions in RocksDB

If you run this, you will see output like this:
```
+++ Merge
+++ Merge
>>> Get value = FullMergeV2
    FullMergeV2 <empty> + 2 merge ops
--- COMPACT
    PartialMergeMulti 2 merge ops      <<<<< I would expect full merge here
+++ Merge
--- COMPACT
    FullMergeV2 <empty> + 2 merge ops
```
I would expect to see `FullMergeV2` after first compaction.

But if we add put operation (uncomment in line 83) then
```
=== Put
+++ Merge
+++ Merge
>>> Get value = FullMergeV2
    FullMergeV2 SOME_VALUE + 2 merge ops
--- COMPACT
    FullMergeV2 SOME_VALUE + 2 merge ops  <<<<< full merge, not partial!
+++ Merge
--- COMPACT
    FullMergeV2 SOME_VALUE + 1 merge ops
```
