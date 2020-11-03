import subprocess

tt = subprocess.run(['sudo','pigpiod'],capture_output=True)
print(f"start daemon : {tt.stdout}")
tt = subprocess.run(['sudo','pigs','t'],capture_output=True)
print(f"check daemon : {tt.stdout}")
tt = subprocess.run(['sudo','killall','pigpiod'],capture_output=True)
print(f"stop daemon : {tt.stdout}")
