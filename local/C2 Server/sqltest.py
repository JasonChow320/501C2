import sqlite3

connection = sqlite3.connect("./data/real.db")
c = connection.cursor()

# cursor.execute('INSERT INTO Implants (authorize_code, agentId, checkTime, IP, sleepTime, guido, computerName, DHkey)\
#                 VALUES ("test", "Yardy", 34, "127.0.0.14", 36, "de", "ez", "nuts")')


#create table
c.execute('''CREATE TABLE Tasks
             (data, data_ip)''')
			
connection.commit()

for row in c.execute("select * from Tasks"):
    print(row)
