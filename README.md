# LiTM
A Lightweight Deterministic Software Transactional Memory System

The directory structure as well as the apps are ported from PBBS (see the README in the app sub folders for details).

Currently we have 6 apps.

+ listRanking
+ maximalIndependentSet
+ maximalMatching
+ pagerank
+ randPerm
+ spanningForest

# Build

Install a gcc compiler with cilkplus support. (Resource here: https://www.cilkplus.org/download-0)

For example, you may simply download the pre-built binary at https://www.cilkplus.org/sites/default/files/cilk-gcc-compiler/cilkplus-4_8-install.tar_0.bz2

Make changes to the environment variables accordingly,

		export LD_LIBRARY_PATH=<your-installation-directory>/cilkplus-4_8-install/lib:<your-installation-directory>/cilkplus-4_8-install/lib64:$LD_LIBRARY_PATH
		export LIBRARY_PATH=<your-installation-directory>/cilkplus-4_8-install/lib:<your-installation-directory>/cilkplus-4_8-install/lib64:$LIBRARY_PATH

Navigate to the $APP$/txn2Phase folder, depend on how you get your cilkplus-gcc, you might need to modify the file *parallelDefs*.

If you use the pre-built cilkplus-gcc binary, you might need to add 

		-lcilkrts -I <your-installation-directory>/cilkplus-4_8-install/include

To **both** the PCFLAGS and PLFLAGS.

Then execute
		make

To have the input files ready, you can navigate to $APP$/graphData and execute
		make

Then go back to $APP$/txn2Phase and execute
		python testInputs

# Contribute

Please report any bugs/problems in the issues tab. Thank you. :)

Pull requests are welcome!
