from A.draft import calculate_prefix_func
from C.draft import calculate_z_function


def max_sub_period(s):
    max_k = 1
    for i in range(len(s)):
        z = calculate_z_function(s[i:])

        for j in range(1, len(z)):
            if z[j] >= j and z[j] % j == 0 and max_k <= z[j] / j + 1:
                max_k = z[j] / j + 1
                max_period = s[i : j + i]

    print(f"Max period string = {max_period}")
    return int(max_k)


if __name__ == "__main__":
    while True:
        s = input()

        print(max_sub_period(s))
