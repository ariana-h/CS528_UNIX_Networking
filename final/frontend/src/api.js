const API_URL = "http://127.0.0.1:5000/api";

export async function fetchMessages() {
    const res = await fetch(`${API_URL}/messages`);
    return res.json();
}

export async function addMessage(content) {
    await fetch(`${API_URL}/add_message`, {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
            "X-API-KEY": "123456789abcdef",
        },
        body: JSON.stringify({ content }),
    });
}

export async function fetchSummaries() {
    const res = await fetch(`${API_URL}/daily_summaries`);
    return res.json();
}
