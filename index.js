var express = require('express');
var app = express();
var bodyParser = require('body-parser');
var path = require('path');

var PORT = process.env.PORT || 2208;


var http = require('http');
// var WebSocket = require('ws');
var server = http.createServer(app);
const  io = require('socket.io')(server);

var ip = require("ip");




app.use('/static',express.static(path.join(__dirname, 'public')))
  .set('views', path.join(__dirname, 'views'))
  .engine('html', require('ejs').renderFile)
  .set('view engine', 'html');

app.use(bodyParser.json());




/////////////////////////////////////////////////////////////////////////////////


/*
var dbConfig = require('./database.config.js');
var mongoose = require('mongoose');

//MONGODB
mongoose.Promise = global.Promise;
mongoose.connect(dbConfig.url, {
});
mongoose.connection.on('error', function() {
    console.log('Could not connect to the database. Exiting now...');
    process.exit();
});
mongoose.connection.once('open', function() {
    console.log("Successfully connected to the database");
});

*/

/////////////////////////////////////////////////////////////////////////////////////
//modelsize 126,789 KB (3756993 Lines)


is_wemos_connected = 0;
io.sockets.on('connection', function (socket) {

    socket.on('connection', function(data){
        io.sockets.emit("message", data);
        console.log(data);
    });


    socket.emit('message','hello from server');


    socket.on('bind_json_data', function (data) {// client emitted
        io.sockets.emit('message', data);//server emittto arduino

        // console.log("\n");console.log(data);console.log("\n");
    });


    socket.on("msg_brc", function(data){
        io.sockets.emit('msg_brc', data);//to cl
    });

    socket.on("wemos_connected", function(data){
        io.sockets.emit('wemos_connected', data);
        is_wemos_connected = 1;
        // console.log("is_wemos_connected:: " + is_wemos_connected);
    });



    socket.on('check_wemos', function (data) {
        io.sockets.emit('check_wemos_feedback', {"is_wemos_connected" : is_wemos_connected});
        console.log("is_wemos_connected:: " + is_wemos_connected);
    });


    socket.on("set_listening_active", function(data){
        io.sockets.emit('message', data);//server emittto arduino
        // console.log("\n");console.log(data);console.log("\n");
    });


    socket.on("demo_action", function(data){
        // console.log(dt);
        io.sockets.emit('demo_action', data);
    });


    socket.on('disconnect', function(data){
        console.log(data);
    });


});







//WSS
/*
const wss = new WebSocket.Server({ server });

wss.on('connection', function connection(ws) {
	console.log('open connection');
    ws.on('message', function incoming(message) {

        wss.clients.forEach(function each(client) {
            if (client !== ws && client.readyState === WebSocket.OPEN) {

                if(message.split("|")[0] == "wms_d1"){
                    // console.log("wss:: "+message.split("|")[0]+", machine::"+message.split("|")[1]);
                }else{
                    if(JSON.parse(message).client_type == 1){
                        // console.log("wss:: Server Broadcast");
                        client.send(message);
                    }
                    if(JSON.parse(message).client_type == 0){
                    }
                }
            }
        });
    });

    // ws.send("APP SERVER");
    ws.on('close', function() {
        console.log('closing connection');
    });

});


wss.broadcast = function broadcast(data) {
    wss.clients.forEach(function each(client) {
        if (client.readyState === WebSocket.OPEN) {
            client.send(data);
        }
    });
};
*/


///////////////////////////////////////////////////////////////////////////////


require('./routes/bones_route.js')(app);


///////////////////////////////////////////////////////////////////////////////













app.get('/custommotion', function(req, res){
    res.render('custom_motion', { title : "Realtime PSO-IK of Humanoid 10 DoF Simulator", name: 'Integrated Socket.io based Three.js, Node.js and Express.js'});
});





app.get('/bootstrap_test', function(req, res){
    dt = {
            title : "Realtime PSO-IK of Humanoid 10 DoF Simulator",
            name: 'Integrated Socket.io based Three.js, Node.js and Express.js',
            content: "Klik The Robot Body Part in The Left Side Bar <br/> Hold (SPACE) for Play and Stop The Animation"
    }
    res.render('bootstrap_test', dt);
});





app.get('/boolean_animation', function(req, res){
    dt = {
            title : "Realtime PSO-IK of Humanoid 10 DoF Simulator",
            name: 'Integrated Socket.io based Three.js, Node.js and Express.js',
            content: "Klik The Robot Body Part in The Left Side Bar \n Hold (SPACE) for Play and Stop The Animation"
    }
    res.render('boolean_animation', dt);
});




app.get('/map_control', function(req, res){
    dt = {
            title : "Realtime PSO-IK of Humanoid 10 DoF Simulator",
            name: 'Integrated Socket.io based Three.js, Node.js and Express.js',
            content: "Hold (W) for Walk, (A) for Turn Left, (D) for Turn Right and Find the Dancing Box Then Hold (Z, X, C) for Dance Action"
    }
    res.render('map_control', dt);
});




app.get('/custom_motion', function(req, res){
    dt = {
            title : "Realtime PSO-IK of Humanoid 10 DoF Simulator",
            name: 'Integrated Socket.io based Three.js, Node.js and Express.js',
            content: "Configure The Robot Motor Rotation and Add to Timeline (Left Side Bar) and Klik Play to Control the Robot"
    }
    res.render('cmot', dt);
});
















///////////////////////////////////////////////////////////////////////////////////





app.get('/', function (req, res) {
    res.render('index', { name: 'Integrated Socket.io based Three.js, Node.js and Express.js'});
});


var products = [
{
    id: 1,
    name: 'laptop'
},
{
    id: 2,
    name: 'microwave'
}
];

var currentId = 2;



app.get('/products', function(req, res) {
    res.send({ products: products });
});

app.post('/products', function(req, res) {
    var productName = req.body.name;
    currentId++;

    products.push({
        id: currentId,
        name: productName
    });

    res.send('Successfully created product!');
});

app.put('/products/:id', function(req, res) {
    var id = req.params.id;
    var newName = req.body.newName;

    var found = false;

    products.forEach(function(product, index) {
        if (!found && product.id === Number(id)) {
            product.name = newName;
        }
    });

    res.send('Succesfully updated product!');
});

app.delete('/products/:id', function(req, res) {
    var id = req.params.id;

    var found = false;

    products.forEach(function(product, index) {
        if (!found && product.id === Number(id)) {
            products.splice(index, 1);
        }
    });

    res.send('Successfully deleted product!');
});





// app.listen(PORT, function() {
//     console.log('Server listening on ' + PORT);
// });

server.listen(PORT, function(){
  console.log('server run on '+ip.address()+":"+server.address().port);//+" "+__dirname);
});
