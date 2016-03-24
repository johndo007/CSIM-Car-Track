import subprocess

f = open('output.txt', 'w')
f2 = open('output2.txt', 'w')

for i in range (1, 61):
    cmd = ['./csim', "0", str(i) + " > output.txt"]
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    process.wait()
    f.write("NUMBER OF CARS WITHOUT LIGHTS: " + str(i) + "\n")
    for line in process.stdout:
        f.write(line)

for i in range (1, 61):
    cmd = ['./csim', "1", str(i) + " > output2.txt"]
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    process.wait()
    f2.write("NUMBER OF CARS WITH LIGHTS: " + str(i) + "\n")
    for line in process.stdout:
        f2.write(line)
