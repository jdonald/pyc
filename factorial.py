def factorial(n):
    if n <= 1:
        return 1
    else:
        return n * factorial(n - 1)

def main():
    result = factorial(5)
    print(result)
    return 0

if __name__ == "__main__":
    main()
