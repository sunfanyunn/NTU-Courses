I implemented the version the server will write 'number of connected client' to the fifo file when receiving SIGUSR1.

//No entry for thread_num since I started with MP3's sample code

Server's config file:

    path=
    account_path=
    run_path=

*Please give absolute path only
 Since I will change working directory when daemonizing before parsing the config file

