import subprocess 

while True:
    try:
        print subprocess.check_output(['python', 'gps2.py'])
    except KeyboardInterrupt:
        break
