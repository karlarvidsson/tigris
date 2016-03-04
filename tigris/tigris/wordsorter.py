

import sys

def main():
	out = open("10k_words.txt", "w")
	
	words = []
	for line in sys.stdin:
		words.append(line)
	
	words.sort()
	for word in words:
		print(word, file=sys.stdout)

	
	
	
	
	
if __name__ == "__main__":
	main()