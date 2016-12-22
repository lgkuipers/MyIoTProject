var http = require("http");
var fs = require("fs");

var index;
var value;
var valueslist;

var getQueryString = function ( field, url ) {
    var href = url ? url : window.location.href;
    var reg = new RegExp( '[?&]' + field + '=([^&#]*)', 'i' );
    var string = reg.exec(href);
    return string ? string[1] : null;
};

fs.readFile('index.html', function (err, data) {
   if (err) {
      return console.error(err);
   }
   index = data.toString();
});

http.createServer( function (request,response) {
  console.log(request.url);
  switch(request.url) {
    case '/':
      valueslist = fs.readFileSync('values.txt').toString();
//      valueslist2 = "10,10, 200,100";
      var valueslist2 = valueslist.replace(/(?:\r\n|\r|\n)/g,",");
      var valuesarray = valueslist2.split(',');
      var points = "";
      for(i=0; i<valuesarray.length;i++) {
        points += i + ',' + valuesarray[i] + ',';
      }
//      console.log(points);
      //response.writeHead( 200, {'Content-Type': 'text/plain'});
      var res = index.replace("World!", value);
      var res2 = res.replace("Poly!", points );
      response.end(res2);
      break;
    case '/update':
      if (request.method == 'POST') {
        request.on('data', function(chunk) {
          var value0 = getQueryString('field1', chunk.toString());
          fs.appendFile('values.txt', value0 + '\n', function (err) {
            if (err) throw err;
            console.log(value0 + ' is saved!');
            value =value0;
          });
        });
        request.on('end', function(chunk) {
          console.log('end');
        });
      };
      break;
    case '/value' :
      valueslist = fs.readFileSync('values.txt').toString();
      var valueslist2 = valueslist.replace(/(?:\r\n|\r|\n)/g,",");
      var valuesarray = valueslist2.split(',');
      var point = valuesarray[valuesarray.length-2];
      console.log('value'+point);
      var levelvalue = point;
      var data = { level :levelvalue };
      var json = JSON.stringify(data);
      response.writeHead( 200, {'Content-Type': 'application/json'});
      response.end(json);
      break;
    case '/menu' :
      menuslist = fs.readFileSync('menus.txt').toString();
      var menuslist2 = menuslist.replace(/(?:\r\n|\r|\n)/g,",");
      var menusarray = menuslist2.split(',');
      var low = 0;
      var high = menusarray.length-2;
      var i = Math.floor(Math.random() * (high - low + 1) + low);
      var menu = menusarray[i];
      console.log('menu '+menu);
      var menuvalue = menu;
      var data = { menu :menuvalue };
      var json = JSON.stringify(data);
      response.writeHead( 200, {'Content-Type': 'application/json'});
      response.end(json);
      break;
  };
}).listen(3000);

console.log('Server running at http://127.0.0.1:3000/'); 