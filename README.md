Tentacool
=========

This is a C++ implementation of a HpFeedBroker.

It offers to ways to fetch user authentication datas: 
 - File
 - MongoDB

####File structure

Each row of the file shows infos about an user. 
Identifier, secret, publish channels and subscribe channels are separated by a semicolon.
Each channel in the publish and subscribe array is separated by commas.

Example:

    user1;s3cr3t;channel1,channel2;channel22;
    user2;p4ssw0rd;channel16;;

Compilation
===========

To compile the project, follow the next instructions:

    //No compilation instructions yet

	
Aldo Mollica

