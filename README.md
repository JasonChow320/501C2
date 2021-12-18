# 501C2
Our final project for CS 501 had us build a C2 framework which would be allow us to inject malware into server and control its actions on the victim machine. In order to do so, we needed to create 3 seperate components which make up a framework. These components are as follows:

## C2
The C2, also known as the command or control center, needs to be able to handle connections from multiple operators, and multiple implants. Our C2 was built using flask in python. Once built, we hosted it on a local machine which was then moved to the public DDNS of cs501kai.ddns.net. 
Our C2 has the following features:
  - Uses MySQL database

## Implant
The implant is a dropper which is too load the malware on to the victim machine. For our implant, we attempted to implement reflective DLL injection as a way to load the malware. Features of our implant include:
  - Checking for ch0nky.txt in the malware
  - Persistence- Adds itself to Registry key “HKEY_CURRENT_USERSoftware\Microsoft\Windows\CurrentVersion\Run”
  - Gets machine guid and name and sends it to c2
  - Request jobs/tasks from c2, executes it, and returns the output of those tasks back to c2
  - Tasks which include but are not limited to:
    - Stealer
    - Powershell command shells

## Client
The client is built with python and is used to communicate with the C2 server. It is able to send GET or POST requests anywhere but at default is set to do this with the C2. Can send steganographic images over to the C2 allowing for messages to be hidden within images.

## Special Feature (Steganography)
The special feature we chose to implement in our final project was Steganography. Steganpgraphy is the action of hiding a message in another message or in our case an image. There are many ways to do this, but we chose to implement it using least significant bit steganography. This method involves taking the RGB matrix of the image and xoring the least siginificant value in it with the ASCII value of the corresponding character in the message. This is done unitl the entire message has been coded into the image. In some instances there is a noticeable difference between the original image and the image with hidden text, therefore we have implemented an RSA encyrption scheme which adds an extra layer of security for the message.

## Built With
+ [Flask](https://flask.palletsprojects.com/en/2.0.x/)
+ [MySQL](https://docs.oracle.com/cd/E17952_01/mysql-8.0-en/index.html)
+ [Python](https://docs.python.org/3/)
+ [C++](https://docs.microsoft.com/en-us/cpp/?view=msvc-170)

## Group Members
Jason Chow (C2, Implants, Steganography)  
Carlos Lopez (C2, Client, Steganography)  
Akshey Nischal (C2, Client, Steganography)  
Jahyung Yun (C2, Client, Implants)  
Alexandre Dennis (C2 & Client)
