def is_prime(n: int) -> bool:
    """
    Checks if the provided number is prime by checking upto the square root
    """
    for k in range(2, int(n ** 0.5) + 1):
        if n % k == 0:
            return False

    return True

def naive_is_primitive_root(n: int, P: int) -> bool:
    """
    Naive version of a primitive root check (n ^ k mod P) = 1 only when k = P - 1

    If you use this version, you will observe a significant slowdown. That's because
    the exponentiation operator below is actually a linear time complexity operation
    internally, making this function O(P^2) and the whole program O(P^3)
    """
    for i in range(1, P - 1):
        if (n ** i) % P == 1:
            return False

    return True

def is_primitive_root(n: int, P: int):
    """
    Slightly optimized version of a primitive root check

    We can reuse the last computed power in each subsequent loop, thus avoiding the
    excessive exponentiation calculations. That makes this function O(P) and the whole
    program O(P^2)
    """

    x = 1
    for _ in range(1, P - 1):
        x *= n
        x %= P

        if x == 1:
            return False

    return True

# The prime number we're testing with
P = 600 * 700 + 1

# Go through all possible roots
for i in range(1, P):
    if is_primitive_root(i, P):
        # And print the ones that are primitive
        print(i)
