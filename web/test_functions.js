async function sha256(message) {
    // encode as UTF-8
    const msgBuffer = new TextEncoder().encode(message);                    

    // hash the message
    const hashBuffer = await crypto.subtle.digest('SHA-256', msgBuffer);

    // convert ArrayBuffer to Array
    const hashArray = Array.from(new Uint8Array(hashBuffer));

    // convert bytes to hex string                  
    const hashHex = hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
    return hashHex;
 }

 var script = document.createElement('script');
 script.src = 'https://code.jquery.com/jquery-3.4.1.min.js';
 script.type = 'text/javascript';
 document.getElementsByTagName('head')[0].appendChild(script);

 let base_url = "norbinet.asuscomm.com";
 let url = "https://" + base_url;
 let socket = null;
 function myFunction() 
 {
    if (socket == null)
    {
       socket = new WebSocket("wss://" + base_url);

       socket.onopen = function(e) {
          alert("[open] Connection established");
          alert("Sending to server");
          socket.send("My name is John");
       };
    }
    else
    {
       socket.send("My name is NOT JOHN");
    }

    socket.onmessage = function(event) {
       alert(`[message] Data received from server: ${event.data}`);
    };

    socket.onclose = function(event) {
    if (event.wasClean) {
       alert(`[close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
    } else {
       // e.g. server process killed or network down
       // event.code is usually 1006 in this case
       alert('[close] Connection died');
    }
    };

    socket.onerror = function(error) {
    alert(`[error] ${error.message}`);
    };
 }

 function login() 
 {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", url + "/login", true);
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.onreadystatechange = function () {
       if (xhr.readyState === 4 && xhr.status === 200) {
          var json = JSON.parse(xhr.responseText);
          //console.log("SID: " + json.sid);
       }
    };
    var encoded_pass = sha256(document.getElementById('login_pass').value).then(pass => {
       var data = JSON.stringify({
          "user": document.getElementById('login_user').value, 
          "pass": pass,
          "obliterate_sessions": document.getElementById('login_obliterate').checked
       });
       xhr.send(data);
    });
 }
 function register() 
 {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", url + "/register", true);
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.onreadystatechange = function () {
       if (xhr.readyState === 4 && xhr.status === 200) {
          console.log("Szuccesz");
       }
    };
    var encoded_pass = sha256(document.getElementById('register_pass').value).then(pass =>
    {
       var data = JSON.stringify({
          "user": document.getElementById('register_user').value, 
          "pass": pass,
          "email": document.getElementById('register_email').value});
       xhr.send(data);
    });

 }
 function testsess()
 {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", url + "/test_session", true);
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.send("{}");
 }