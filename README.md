# Socket-Programming-Project

# Description

This is my EE450 socket programming project solution.

1 client and 3 servers communicate with each other to calculate network delays.

# Files

`common.hpp`: A header file containing commonly used classes.
`serverA.cpp`: Server A dedicated codes.
`serverB.cpp`: Server B dedicated codes.
`aws.cpp`: Main server dedicated codes.
`client.cpp`: Client dedicated codes.

# Idiosyncrasy

None.

# Exchange Format

Classes for exchange between hosts are able to automatically encode its fields as field length (in bytes) followed by field data.
No space optimization or compression or error check is used.
All data can fit within one packet and communications strictly follow given order, so no sequencing is used.

## client to main server

Fields of class ClientQuery.
Containing Map ID, source vertex index and file size.

## main server to server A

Fields of class `ClientQuery`.
Although, the file size field is useless for server A, I just reused it for simplicity.

## server A to main server

Fields of class `AllShortestPath`.
Containing Map ID, propagation speed, transmission speed, source vertex index and shortest distances.
Although, Map ID is not unnecessary here, I keep it for better data organization.

## main server to server B

Fields of class `ClientQuery` and `AllShortestPath`.
Although, the Map ID and source vertex index fields are useless for server B, I just reused these classes for simplicity.

## server B to main server

Fields of class `AllDelay`.
Containing all propagation delay and transmission delay results.
The end-to-end delay is not stored because it can be easily calculated using `Delay::Total()`.

## main server to client

Fields of class `Response`.
Containing a list of results with all result fields.
The end-to-end delay is not stored because it can be easily calculated using `Delay::Total()`.

# Reused Code

None.
