/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

var recording = false;
var isInitialized = false;
var encoder = new GIFEncoder();
var canvas;
var context;
var context2;
var dataurl_array = new Array();
var dataurl_array_index = 0;

function setStringOnGifProgress(str){
    var test = document.getElementById("gif_progress");
    test.innerText = str;
}

function startRecording(){
    setStringOnGifProgress("Recording Now");
    if (!document.getElementById('execTrace').disabled){
        document.getElementById('execTrace').click();
    }

    if (!isInitialized){
    }
    canvas = document.getElementById('glcanvas');
    context = canvas.getContext("experimental-webgl", {preserveDrawingBuffer: true});

    var canvas2 = document.getElementById('glcanvas2');
    canvas2.style.display= "none";
    canvas2.width = canvas.width;
    canvas2.height = canvas.height;
    console.log("canvas.height, canvas.width = " + canvas.height + " " + canvas.width);
    context2 = canvas2.getContext('2d');
//    console.log(context2);

    isInitialized = true;
    encoder.setRepeat(0); //0  -> loop forever
                        //1+ -> loop n times then stop
    encoder.setDelay(33); //go to next frame every n milliseconds
    encoder.setSize(canvas2.width, canvas2.height);
    console.log("result of encoder.start = " + encoder.start());

    dataurl_array = new Array();
    dataurl_array_index = 0;
    //    context = canvas.getContext("experimental-webgl", {preserveDrawingBuffer: true});
}

function addFrameToEncoder(){
    if (!document.getElementById('execTrace').disabled){
        document.getElementById("myRecord").click();
        return;
    }

//    console.log("result of encoder.addFrame(context) = " + encoder.addFrame(context2));
//    context = canvas.getContext('2d');
//    console.log(context);
   dataurl = renderer.domElement.toDataURL();
    //console.log(dataurl);
   dataurl_array.push(dataurl);
   

}

function recordFrames(){
    setStringOnGifProgress("Generationg Gif: " + (dataurl_array_index / dataurl_array.length * 100).toFixed(3) + " % finished (" + dataurl_array_index + " / " + dataurl_array.length +  ")");

    if (dataurl_array.length == dataurl_array_index){
        saveGif();
        return;
    }
    console.log(dataurl_array.length);
    var img = new Image;
    img.onload = function(){
        context2.drawImage(img,0,0); // Or at whatever offset you like
        encoder.addFrame(context2);
        dataurl_array_index += 1;
        recordFrames();
    }
    img.src = dataurl_array[dataurl_array_index];
}

function finishRecording(){
    recordFrames();
}

function saveGif(){
    encoder.finish();
    encoder.download("download.gif");  
}

function initMyRecoder(){
    var obj2 = document.getElementById("myRecord");

   
    //直接関数を記述して登録する
    obj2.addEventListener( "click" , function () {
        var button = document.getElementById("myRecord");
        recording = !recording;
        if (recording){
            button.value = "Stop Recording"
            startRecording();
        } else {
            finishRecording();
            button.value = "Start Recording"
        }
       
    } , false );
}
  

