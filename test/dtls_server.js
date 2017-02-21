var dtls = require( 'node-dtls' );
var fs = require( 'fs' );

var pem = fs.readFileSync( 'test/cert.pem' );

var server = dtls.createServer({ type: 'udp4', key: pem, cert: pem });
server.bind( 8746 );

server.on( 'secureConnection', function( socket ) {

  console.log( 'New connection from ' +
    [ socket.rinfo.address, socket.rinfo.port ].join(':') );

  socket.on( 'message', function( message ) {
    console.log( message );
    // Echo the message back
    // socket.send( message );
  });
});
