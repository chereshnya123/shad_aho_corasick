from C.draft import calculate_z_function

def find_max_period(s):
  z_f = calculate_z_function(s)
  
  for i, value in enumerate(z_f):
    if value + i >= len(s) and i != 0:
      return int(len(s) / i)
  
  return 1
  
if __name__ == '__main__':
  while True:
    s = input()
    
    print(find_max_period(s))