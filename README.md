# `RIQ`: RDF Indexing on Quadruples

## Summary

`RIQ` is a new approach for fast processing of SPARQL queries on large
RDF quadruples (or quads). `RIQ` employs a *decrease-and-conquer*
strategy: Rather than indexing the entire RDF dataset, `RIQ` identifies
groups of similar RDF graphs and indexes each group separately. During
query processing, `RIQ` uses novel filtering index to first identify
candidate groups that may contain matches for the query. On these
candidates, it executes optimized queries using a conventional SPARQL
processor to produce the final results.

## Publications

* Vasil Slavov, Anas Katib, Vinutha Nuchimaniyanda, Praveen Rao. ``A
Novel Tool for Fast Processing of SPARQL Queries on RDF Quadruples."
**Submitted to** *The 2015 ACM SIGMOD International Conference on Management
of Data* (**SIGMOD 2015**), Melbourne, Australia. (Demo)

* Vasil Slavov, Anas Katib, Praveen Rao, Srivenu Paturi, Dinesh
Barenkala. ``Fast Processing of SPARQL Queries on RDF Quadruples."
[*Proceedings of the 17th International Workshop on the Web and
Databases*](http://webdb2014.eecs.umich.edu/) (**WebDB 2014**),
Snowbird, UT, June 2014.
([manuscript](http://v.web.umkc.edu/vsfgd/files/riq-webdb-paper.pdf),
[technical report](http://v.web.umkc.edu/vsfgd/files/riq-tr.pdf))
