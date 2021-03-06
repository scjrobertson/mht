# README #

## What is this repository for? ##

* Code for the Multiple Hypothesis Tracking (MHT) approach for **EMSS**'s **xRange** project. Implemented using Probabilistic Graphical Models (PGM)s and the **emdw** code base.

## How do I get set up? ##

* You must have been permitted access to **gLinear**, **patrecII** and **emdw** and installed them following the recommended instructions. For now, **emdw** will require some extra hackery to work:

        cd /usr/local/lib
        sudo -ln -sf /home/<your_linux_user_name>/bin/libemdw.so
        sudo ldconfig -v
    
    This assumes you are running some **debian** fork, using anything else would be morally wrong.
    
* Build the project using **cmake**:

         mkdir build; cd $_
         cmake ../; make -j4
         cd src

   Then run which ever binary files were created. **build/test** will contain the output of the unit tests, which is unimaginably exciting. 
   
## API ##
The code is documented using the **JavaDoc** style and the API can be extracted using **doxygen**. 

* First install **doxygen**:

         sudo apt-get install doxygen

* Then extract the API as follows:

         mkdir doc
         doxygen DoxyFile

The output should be some **.html** files and **.tex** files, found in their respective subfolders. The API can now be viewed comfortably without having to sift through all the code.

## Who do I talk to? ##

SCJ Robertson

**Contact**
* robertsonscj@gmail.com
* 16579852@sun.ac.za
