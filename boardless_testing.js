const express = require('express');
const app = express();
const expressWs = require('express-ws')(app);

const PORT = 8000;

app.use(function (req, res, next) {
  req.testing = 'testing';
  return next();
});
 
app.use(express.static(__dirname + '/website')); //sets all static file calls to 

app.get('/',(req,res) => {
  res.sendFile(path.join(__dirname + '/website/index.html'));
});

function isJSON(str) {
    try {
        return (JSON.parse(str) && !!str);
    } catch (e) {
        return false;
    }
}

app.ws('/', function(ws, req) {
  ws.on('message', function(msg) {
    console.log(msg);
    if (isJSON(msg)) {
    	ws.send(msg);
    }
  });
  console.log('socket', req.testing);
});
 
app.listen(PORT, function () {
  console.log('Ready at http://localhost:' + PORT);
});