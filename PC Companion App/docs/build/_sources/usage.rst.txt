Usage
=====

.. _installation:

Run app_main.exe
----------------

Create a new folder, and put app_main.exe inside, run the file, and there you go!
The app will also create a log file in the same folder it is run from.


.. _run script:

Run as python script
--------------------

.. note::
        You need Python >= 3.9 installed on your system in order to do this.

Make sure you have the required packages by entering the folloing in a command line terminal:

>>> pip install -r path/to/PC Companion App/src/requirements.txt

Now just run app_main.py and that's it.


Bundle the python script using pyinstaller
------------------------------------------

Do all the steps in :ref:`Run as python script <run script>`.

Now, make sure you have pyinstaller by running the following command in the command line terminal:

>>> pip install -m pyinstaller

And then:

>>> python pyinstaller --noconsole  --onefile path/to/PC Companion App/src/app_main.py

Now, your app_main.exe should be in ../PC Companion App/dist/ folder. Now you can run it.