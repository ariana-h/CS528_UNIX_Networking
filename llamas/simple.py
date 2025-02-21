#!/usr/bin/env python3

from ollama import Client

model_name = "llama2"
host_addr = "localhost"

def main():
    client = Client(host=f'http://{host_addr}:11434')

    while True:
        prompt = input(f"{model_name}: ")
        if prompt.lower() == 'exit':
            print("Goodbye!")
            break

        try:
            response = client.chat(model=model_name, messages=[{'role': 'user', 'content': prompt}])
            if 'message' in response:
                print(f"{model_name}: {response['message']['content']}")
            else:
                print("Unexpected response format:", response)
        except Exception as e:
            print(f"Error: {e}")

if __name__ == "__main__":
    main()

