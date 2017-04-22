In order to run the code you must complete the following steps:

- decompress the directory
- install boost. This can be done as follows in Linux:
  sudo apt-get install libboost-all-dev
- change directory to the decompressed directory
- run the Makefile as follows:
  sh Makefile.sh
- Before running the code, you will need to set an environment variable as 
  as follows:
  export NEDC_NFC="/path/to/the/code/directory"
  e.g.: If my code is in /home/my_user/decompressed_dir, I will set the var
  	like this: 
	NEDC_NFC="/home/my_user/decompressed_dir"
  You can also add this line to the .bashrc file so you do not have to add it 
  every time you start a new session.
- you can also optionally add the bin/ directory to your path. Running the 
  utilities with the -h option will display a help file:
  
  nedc_print_header -h
