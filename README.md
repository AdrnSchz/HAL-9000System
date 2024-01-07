# HAL-9000System

## Overview
This project implements the HAL 9000 System, consisting of a Discovery Server, a Poole Server, and a Bowman Client. The Discovery manages Bowman connections that will connect to the Poole Server, which manages songs, playlists, and downloads.

## Project Structure
* **Discovery Server:** Manages connections from Poole and Bowman. Configuration is provided by configD.dat.

* **Poole Server:** Handles connections from Bowmans and manages requests from the connected clients. Configuration is provided by configP.dat. 
>Example: Song and playlist information is stored in the smyslov (name of the Server stored in the configuration file) folder.

* **Bowman Client:** Connects to the Poole Server to be able to make specific requests. Configuration is provided by configB.dat.
>Example: Downloads are stored in the floyd (name of the Client stored in the configuration file) folder.

## Configuration Files
* configD.dat: Configuration file for the Discovery Server.
* configP.dat: Configuration file for the Poole Server.
* configB.dat: Configuration file for the Bowman Client.

* configP2.dat: Configuration file for the Poole Server.
* configP3.dat: Configuration file for the Poole Server.
* configB2.dat: Configuration file for the Bowman Client.
* configB3.dat: Configuration file for the Bowman Client.
* configB4.dat: Configuration file for the Bowman Client.

## Data Organization
### floyd Folder: 
* Contains downloads for floyd Bowman client.
### smyslov Folder:
* playlists.txt: Information about playlists available on the Poole Server.
* songs.txt: Information about all songs available on the Poole Server.
* MP3 files: Actual song files.

#### Other directories correspond to the other Pooles and Bowmans.


## How to Run
* make
* run Discovery Server -> $ discovery configD.dat
* run Poole -> $ poole configP.dat
* run Bowman -> $ bowman configP.dat

This can be done for multiple Pooles and Bowmans with the other configuration files.



