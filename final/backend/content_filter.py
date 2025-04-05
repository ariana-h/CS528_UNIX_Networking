import ollama

def query_llama(prompt: str) -> str:
    response = ollama.chat(
        model='llama3.2',
        messages=[
            {"role": "system", "content": "You are a content safety classifier."},
            {"role": "user", "content": prompt}
        ]
    )
    return response['message']['content'].strip().lower()

def is_misinformation(text: str) -> bool:
    prompt = f"""Determine if the following statement contains likely misinformation.

Respond with only "yes" or "no".

Statement: "{text}"
"""
    answer = query_llama(prompt)
    return answer.startswith("yes")

def is_offensive(text: str) -> bool:
    prompt = f"""Determine if the following statement is offensive, hateful, or abusive.

Respond with only "yes" or "no".

Statement: "{text}"
"""
    answer = query_llama(prompt)
    return answer.startswith("yes")
