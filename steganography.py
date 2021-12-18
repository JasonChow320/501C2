import cv2
import string
import os

#Picks where to encrypt string
kl = 0
x = 0  # number of rows
y = 0  # number of columns
z = 0  # number of applicates (not sure if this is right, but I was told it is)

uniDicc = {}
reverseUni = {}

for idx in range(255):
    uniDicc[chr(idx)] = idx
    reverseUni[idx] = chr(idx)
    
def encrypt(pic,key, text, x=0,y=0,z=0,kl=0):
    for i in range(len(text)): #loops through lenght of text
        pic[x, y, z] = uniDicc[text[i]] ^ uniDicc[key[kl]] 
        x = x+1
        y = y+1
        y = (y+1) % 3 #our for loop alters pixels in a vertical pattern
        kl = (kl+1) % len(key)  
    return pic
    
try:
    OGname= str(input("Which image would you like to alter?:"))
    OGpic = cv2.imread(OGname)
except:https://www.youtube.com/watch?v=Ais5yNGSvHM&ab_channel=PyPowerProjects
    print("image not found")

SafeKey = input("Enter key to edit(Security Key): ")
EncText = input("Enter text to hide: ")
tlen= len(EncText)
EncPic = encrypt(OGpic, SafeKey, EncText)

cv2.imwrite(OGname, EncPic)
print("Data hidden in image.")

def decrypt(pic,key, msglen, x=0,y=0,z=0, kl=0):
    answer = ""
    for i in range(msglen): #for loop to traverse the length of message
        answer += reverseUni[pic[x, y, z] ^ uniDicc[key[kl]]]
        x = x+1
        y = y+1
        y = (y+1) % 3
        kl = (kl+1) % len(key) #based on length retrieves originals pixels and extracts message
    return answer

key1 = input("\nRe Enter key to extract text : ")
if SafeKey == key1:
    print("Encrypted text was : ", decrypt(EncPic, SafeKey, tlen))
else:
    print("Wrong key, ya dingus.")