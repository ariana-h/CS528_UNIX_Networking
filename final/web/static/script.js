async function fetchMessages() {
    try {
        const res = await fetch('/api/messages');
        const data = await res.json();

        const container = document.getElementById('messages');
        container.innerHTML = '';

        data.forEach(msg => {
            const div = document.createElement('div');
            div.className = 'message';
            div.textContent = msg;
            container.appendChild(div);
        });
    } catch (err) {
        console.error('Error fetching messages:', err);
    }
}

fetchMessages();
setInterval(fetchMessages, 5000);