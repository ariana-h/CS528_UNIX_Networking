#!/usr/bin/env python3

from ollama import Client

model_name = "llama3.2"
host_model = "llama3.1"
host_addr = "localhost"

def main():
    client = Client(host=f'http://{host_addr}:11434')
    host = Client(host=f'http://localhost:11434')

    response = None
    prompt = input(f"{model_name}: ")
    while True:
        # Talk to other machine
        try:
            if response is None:
                response = client.chat(model=model_name, messages=[{'role': 'user', 'content': prompt}])
            else:
                response = client.chat(model=model_name, messages=[{'role': 'user', 'content': str(response)}])
            if 'message' in response:
                print(f"{model_name}: {response['message']['content']}")
                response = f"{response['message']['content']}"
            else:
                print("Unexpected response format:", response)
        except Exception as e:
            print(f"Error: {e}")

        # Talk to myself
        try:
            response = host.chat(model=host_model, messages=[{'role': 'user', 'content': str(response)}])
            if 'message' in response:
                print(f"{host_model}: {response['message']['content']}")
                response = f"{response['message']['content']}"
            else:
                print("Unexpected response format:", response)
        except Exception as e:
            print(f"Error: {e}")
        


if __name__ == "__main__":
    main()

