const dgram = require('dgram');
const message = Buffer.from('Some bytes');
const client = dgram.createSocket('udp4');
for( let i = 0; i < 300; ++i )
    client.send(message, 41234, 'localhost', (err) => {} );
// client.send(message, 41234, 'localhost', (err) => {
//   client.close();
// });
