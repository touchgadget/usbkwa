static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang=en>
<head>
<meta charset="UTF-8">
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>USB Touch keyboard</title>
<style type="text/css">
    table {
        position: absolute;
        top: 0;
        bottom: 0;
        left: 0;
        right: 0;
        height: 100%;
        width: 100%;
        border-collapse: collapse;
    }

    td {
        width: 5.88%;
        border: 1px solid;
        font-size: 150%;
        text-align: center;
    }
</style>
<script type="text/javascript">
var websock;
var connected = false;
let gridData = [
  [ // Keyboard row 0 (top row)
    { cellLabel: "Esc", colSpan: 2},
    { cellLabel: "F1"},
    { cellLabel: "F2"},
    { cellLabel: "F3"},
    { cellLabel: "F4"},
    { cellLabel: "F5"},
    { cellLabel: "F6"},
    { cellLabel: "F7"},
    { cellLabel: "F8"},
    { cellLabel: "F9"},
    { cellLabel: "F10"},
    { cellLabel: "F11"},
    { cellLabel: "F12"},
    { cellLabel: ""},
    { cellLabel: "Prt<br>Scr"},
    { cellLabel: "Scroll<br>Lock"},
    { cellLabel: "Pause<br>Break"},
  ],
  [ // Keyboard row 1
    { cellLabel: "~<br>`"},
    { cellLabel: "!<br>1"},
    { cellLabel: "@<br>2"},
    { cellLabel: "#<br>3"},
    { cellLabel: "$<br>4"},
    { cellLabel: "%<br>5"},
    { cellLabel: "^<br>6"},
    { cellLabel: "&<br>7"},
    { cellLabel: "*<br>8"},
    { cellLabel: "(<br>9"},
    { cellLabel: ")<br>0"},
    { cellLabel: "_<br>-"},
    { cellLabel: "+<br>="},
    { cellLabel: "Backspace", colSpan: 2},
    { cellLabel: "Insert"},
    { cellLabel: "Home"},
    { cellLabel: "Page<br>Up"},
  ],
  [ // Keyboard row 2
    { cellLabel: "Tab", colSpan: 2},
    { cellLabel: "Q"},
    { cellLabel: "W"},
    { cellLabel: "E"},
    { cellLabel: "R"},
    { cellLabel: "T"},
    { cellLabel: "Y"},
    { cellLabel: "U"},
    { cellLabel: "I"},
    { cellLabel: "O"},
    { cellLabel: "P"},
    { cellLabel: "{<br>["},
    { cellLabel: "}<br>]"},
    { cellLabel: "|<br>\\"},
    { cellLabel: "Delete"},
    { cellLabel: "End"},
    { cellLabel: "Page<br>Down"},
  ],
  [ // Keyboard row 3
    { cellLabel: "Caps<br>Lock", colSpan: 2},
    { cellLabel: "A"},
    { cellLabel: "S"},
    { cellLabel: "D"},
    { cellLabel: "F"},
    { cellLabel: "G"},
    { cellLabel: "H"},
    { cellLabel: "J"},
    { cellLabel: "K"},
    { cellLabel: "L"},
    { cellLabel: ":<br>;"},
    { cellLabel: "\"<br>\'"},
    { cellLabel: "Enter", colSpan: 2},
    { cellLabel: ""},
    { cellLabel: ""},
    { cellLabel: ""},
  ],
  [ // Keyboard row 4
    { cellLabel: "Shift", colSpan: 2},
    { cellLabel: "Z"},
    { cellLabel: "X"},
    { cellLabel: "C"},
    { cellLabel: "V"},
    { cellLabel: "B"},
    { cellLabel: "N"},
    { cellLabel: "M"},
    { cellLabel: "\<<br>,"},
    { cellLabel: "\><br>."},
    { cellLabel: "?<br>/"},
    { cellLabel: "Shift", colSpan: 3},
    { cellLabel: ""},
    { cellLabel: "Up<br>Arrow"},
    { cellLabel: ""},
  ],
  [ // Keyboard row 5
    { cellLabel: "Ctrl", colSpan: 2},
    { cellLabel: "GUI<br>Alt<br>Option"},
    { cellLabel: "Alt<br>GUI<br>Clover"},
    { cellLabel: "Space", colSpan: 5},
    { cellLabel: "Alt<br>GUI<br>Clover", colSpan: 2},
    { cellLabel: "GUI<br>Alt<br>Option"},
    { cellLabel: "Context<br>Menu"},
    { cellLabel: "Ctrl", colSpan: 2},
    { cellLabel: "Left<br>Arrow"},
    { cellLabel: "Down<br>Arrow"},
    { cellLabel: "Right<br>Arrow"},
  ]
];

function touch_start(event) {
    event.preventDefault();
    e = document.getElementById(this.id);
    e.style.backgroundColor = "yellow";
    var json = JSON.stringify({event:'touch start', name:this.innerHTML, row:this.row, col:this.col});
    if (!connected) {
        setTimeout(function(json) {
            websock.send(json);
        }, 200);
        window.location.reload();
    }
    else {
        websock.send(json);
    }
}
function touch_end(event) {
    event.preventDefault();
    e = document.getElementById(this.id);
    e.style.backgroundColor = "white";
    websock.send(JSON.stringify({event:'touch end', name:this.innerHTML, id:this.id, row:this.row, col:this.col}));
}
function touch_move(event) {
    event.preventDefault();
    var json = JSON.stringify({event:'touch move', name:this.innerHTML, row:this.row, col:this.col});
    if (!connected) {
        setTimeout(function(json) {
            websock.send(json);
        }, 200);
        window.location.reload();
    }
    else {
        websock.send(json);
    }
}
function touch_cancel(event) {
    event.preventDefault();
    document.getElementById(this.id).style.backgroundColor = "white";
    websock.send(JSON.stringify({event:'touch cancel', name:this.innerHTML, id:this.id, row:this.row, col:this.col}));
}
function button_click(event) {
    event.preventDefault()
    //document.getElementById(this.id).style.backgroundColor = "yellow";
}
function double_click(event) {
    event.preventDefault();
    console.log('double click', this.id, this.innerHTML, event);
}
function context_menu(event) {
    event.preventDefault();
    console.log('right click', this.id, this.innerHTML, event);
}
function generateGrid(cols, rows, cellData, id) {
    let grid = document.getElementById(id);
    let r = 0;
    for (let rowElement of cellData) {
        let aRow = grid.insertRow();
        let c = 0;
        for (let colElement of rowElement) {
            let cell = aRow.insertCell();
            cell.id = 'r' + r + 'c' + c;
            cell['row'] = r;
            cell['col'] = c;
            cell.onclick = button_click;
            cell.ondblclick = double_click;
            cell.oncontextmenu = context_menu;
            cell.ontouchstart = touch_start;
            cell.onmousedown = touch_start;
            cell.ontouchmove = touch_move;
            cell.onmousemove = touch_move;
            cell.ontouchend = touch_end;
            cell.onmouseup = touch_end;
            cell.onmouseout = touch_end;
            cell.onmouseleave = touch_end;
            cell.ontouchcancel = touch_cancel;
            cell.innerHTML = colElement["cellLabel"];
            if (colElement["colSpan"] > 1) {
                cell.colSpan = colElement["colSpan"];
            }
            c++;
        }
        r++;
    }
}

var FullPage = document.documentElement;
function openFullscreen() {
    if (FullPage.requestFullscreen) {
        FullPage.requestFullscreen();
    } else if (FullPage.mozRequestFullScreen) { /* Firefox */
        FullPage.mozRequestFullScreen();
    } else if (FullPage.webkitRequestFullscreen) { /* Chrome, Safari & Opera */
        FullPage.webkitRequestFullscreen();
    } else if (FullPage.msRequestFullscreen) { /* IE/Edge */
        FullPage.msRequestFullscreen();
    }
}

function start() {
  generateGrid(17, 6, gridData, "my_table");
  websock = new WebSocket('ws://' + window.location.hostname + ':81/');
  websock.onopen = function(evt) { console.log('websock onopen', evt); connected = true; };
  websock.onclose = function(evt) { console.log('websock onclose', evt); connected = false; };
  websock.onerror = function(evt) { console.log('websock onerror', evt); };
  websock.onmessage = function(evt) { console.log(evt); };
}

</script>
</head>
<body onload="javascript:start();" onresize="window.location.reload();">
<table id="my_table">
<!-- Grid goes here -->
</table>
</body>
</html>
)rawliteral";
