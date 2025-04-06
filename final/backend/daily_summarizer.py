import requests, datetime, ollama, os, sys

API_URL   = os.getenv("API_URL",   "http://localhost:5000")
API_KEY   = os.getenv("API_KEY",   "123456789abcdef")
OLLAMA_MODEL = os.getenv("OLLAMA_MODEL", "llama3.2")

def fetch_messages():
    r = requests.get(f"{API_URL}/api/messages")
    r.raise_for_status()
    return r.json()

def summarise(messages):
    if not messages:
        return "No messages were posted today."

    joined = "\n".join(f"- {m['content']}" for m in messages)
    response = ollama.chat(
        model=OLLAMA_MODEL,
        messages=[
            {"role": "system",
             "content": "You are an assistant that writes a SHORT, clear daily digest. This is so the audience understands what has been flagged for misinformation or violence. If the day's messages contain offensive or misinformation, briefly explain why."
                        "Audience: Customers interested in what happened that day. Length: ≤ 150 words."},
            {"role": "user", "content": f"Summarise today's messages:\n{joined}"}
        ],
    )
    return response["message"]["content"].strip()

def store_summary(summary):
    payload = {
        "date": datetime.date.today().isoformat(),
        "content": summary
    }
    r = requests.post(  
        f"{API_URL}/api/daily_summary",
        json=payload,
        headers={"X-API-KEY": API_KEY},
        timeout=10,
    )
    r.raise_for_status()

def delete_originals():
    today = datetime.date.today().isoformat()
    r = requests.delete(
        f"{API_URL}/api/delete_messages_before/{today}",
        headers={"X-API-KEY": API_KEY},
        timeout=10,
    )
    r.raise_for_status()
    return r.json().get("deleted", 0)

def main():
    msgs = fetch_messages()
    summary = summarise(msgs)
    store_summary(summary)
    deleted = delete_originals()
    print(f"Stored summary ({len(summary)} chars). Deleted {deleted} old messages.")
    print(f"Summary: {summary}")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"Daily summariser failed: {e}", file=sys.stderr)
        sys.exit(1)