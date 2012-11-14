pippolo
=======

Pippolo is a very young and simple distributed database completely written in C. The entire system working like a graph: multiple nodes cooperate to keep informations synchronised and available even if one (or more, depends how the system was configured) is down. Each recall (insertion, delete or get) from a client connected at any one node of the graph is a XML data block, so, it is language-abstract. The powerful of this tool coming from the ability to extend and modify the graph while the system is online.