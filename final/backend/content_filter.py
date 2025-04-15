import json
import ollama

SYSTEM_PROMPT = "You are a content‑safety classifier. Reply in JSON only."


def _query(category: str, text: str):
    """Return (flag: bool, explanation: str) for a single category."""
    json_shape = "{{\"flag\": \"yes|no\", \"explanation\": \"<one‑sentence explanation>\"}}"

    prompt = (
        f"For the following statement decide whether it is {category}.\n"
        f"Reply in exactly this JSON shape:\n{json_shape}\n\n"
        f"Statement: \"{text}\""
    )

    response = ollama.chat(
        model="llama3.2",
        messages=[
            {"role": "system", "content": SYSTEM_PROMPT},
            {"role": "user", "content": prompt},
        ],
    )

    raw = response["message"]["content"].strip()

    try:
        data = json.loads(raw)
    except json.JSONDecodeError:
        flag_line, *rest = raw.splitlines()
        data = {
            "flag": flag_line.strip().lower(),
            "explanation": (" ".join(rest)).strip(),
        }

    flag_bool = str(data.get("flag", "")).lower().startswith("yes")
    explanation = data.get("explanation", "").strip()
    return flag_bool, explanation


def check_misinformation(text: str):
    """Return (is_misinformation: bool, explanation: str)."""
    return _query("likely misinformation", text)


def check_offensive(text: str):
    """Return (is_offensive: bool, explanation: str)."""
    return _query("offensive, hateful or abusive", text)

def is_misinformation(text: str) -> bool:
    flag, _ = check_misinformation(text)
    return flag


def is_offensive(text: str) -> bool:
    flag, _ = check_offensive(text)
    return flag
