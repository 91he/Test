var oldObj = [], oldWidth = 0, oldHeight;
var curId = 0, restrictHeight = 140;

function stopBubble(obj, e){
    var width = document.body.offsetWidth;
    var index = parseInt(obj.getAttribute("index"), 10);
    var imageShower = document.getElementById("image_shower");

    if(e.clientX < width / 2){
        if(index - 1 >= 0) imageShower.children[index - 1].firstElementChild.click();
    }else if(index + 1 < imageShower.children.length){
        imageShower.children[index + 1].firstElementChild.click();
    }

    if (e && e.stopPropagation)
        e.stopPropagation();
    else
        window.event.cancelBubble = true;
}

function getScrollbarWidth() {
    var oP = document.createElement('p'),
        styles = {
            width: '100px',
            height: '100px',
            overflowY: 'scroll'
        }, i, scrollbarWidth;
    for (i in styles) oP.style[i] = styles[i];
    document.body.appendChild(oP);
    scrollbarWidth = oP.offsetWidth - oP.clientWidth;
    oP.remove();
    return scrollbarWidth;
}

function time(){
    return new Date().getTime();
}


function scrollFunc(){
    if(window.innerHeight + document.body.scrollTop > document.body.scrollHeight - 200){
        genEmiter();
    }
}

function hideFunc(){
    var layer = document.getElementById('layer');

    layer.removeAttribute("src");
    layer.removeAttribute("width");
    layer.removeAttribute("height");
    document.getElementById('fix_bg').style.height = "0px";
    document.getElementById('fix_layer').style.height = "0px";
}

function genEmiter(){
    var xmlhttp = new window.XMLHttpRequest();
    xmlhttp.onreadystatechange = function(){
        if(xmlhttp.readyState == 4 && xmlhttp.status == 200){
            var obj = JSON.parse(xmlhttp.responseText);
            if(obj.length){
                curId += obj.length;
                gen(obj);
            }
        }
    }
    xmlhttp.open("GET", "/api/images/" + curId, true);
    xmlhttp.send();
}

function makeDiv(index, path, width, height, image_width, image_height){
    var showAble;
    var div = document.createElement("div");
    var img = document.createElement("img");

    //console.log(width + "," + height + "," + image_width + "," + image_height);
    //console.log(path + "," + image_width + "," + image_height);
    div.style.display = "inline-block";
    div.style.verticalAlign = "top";
    div.style.width = width + "px";
    div.style.height = height + "px";
    div.setAttribute("hw", image_width);
    div.setAttribute("hh", image_height);
    //div.style.backgroundColor = "#" + Math.round(Math.random() * 0xfff).toString(16);

    if(width / height > image_width / image_height){
        img.height = height;
        img.width = Math.round(height * image_width / image_height);
        div.style.width = img.width + "px";
        div.style.paddingLeft = Math.round((width - img.width) / 2);
        //div.style.marginLeft = Math.round((width - img.width) / 2);
    }else{
        img.width = width;
        img.height = Math.round(width * image_height / image_width);
        div.style.height = img.height + "px";
        div.style.paddingTop = Math.round((height - img.height) / 2);
        //div.style.marginTop = Math.round((height - img.height) / 2);
    }

    img.src = path;
    img.style.transition = "transform .3s";
    img.onmouseover = function(){
        showAble = true;
        setTimeout(function(){
            if(!showAble) return;
            var tx = 0, ty = 0;
            var showerWidth = div.parentElement.offsetWidth;
            showerWidth -= window.innerHeight == document.body.scrollHeight ? getScrollbarWidth() : 0;

            if(img.width > width / 2){
                if(div.offsetLeft == 0)
                    tx += img.width - width / 2;
                else if(div.offsetLeft + width == showerWidth)
                    tx -= img.width - width / 2;
            }
            
            if(img.height > height / 2){
                if(div.offsetTop == 0)
                    ty += img.height - height / 2;
                else if(div.offsetTop + height == div.parentElement.offsetHeight)
                    ty -= img.height - height / 2;
            }
            
            img.style.transform = "translate(" + tx + "px, " + ty + "px) scale(2)"
            img.style.boxShadow = "0px 0px 10px #000";
        }, 300);

    };
    img.onmouseout = function(){showAble = false; img.style.transform = ""; img.style.boxShadow = "";};
    img.onclick = function(){
        var w = document.body.offsetWidth;
        var h = window.innerHeight;
        var fix_bg = document.getElementById("fix_bg");
        var fix_layer = document.getElementById("fix_layer");
        var layer = document.getElementById("layer");

        fix_bg.style.height = h + "px";
        fix_layer.style.height = h + "px";
        layer.removeAttribute("width");
        layer.removeAttribute("height");

        if(w / h > image_width / image_height){
            if(image_height > h) layer.height = h;
        }else if(image_width > w){
            layer.width = w;
        }

        layer.setAttribute("index", index);
        layer.src = img.src;
    };

    div.appendChild(img);

    return div;
}

function gen(obj){
    var begin = 0, end;
    var ratioArr = [], ratioCur = 0, ratioLast;
    var imageShower = document.getElementById("image_shower");
    var showerWidth = imageShower.offsetWidth > 1000 ? imageShower.offsetWidth : 1000;
    var count = imageShower.children.length;

    if(window.innerHeight == document.body.scrollHeight){
        showerWidth -= getScrollbarWidth();
    }

    //console.log(showerWidth);
    obj = oldObj.concat(obj);
    for(var index = 0; index < obj.length; index++){
        var ratio = obj[index].width / obj[index].height;
        ratioArr[index] = ratio > 2 ? 2 : ratio;
        ratioLast = ratioCur; 
        ratioCur += ratioArr[index];
        //ratioArr[index] += index == 0 ? ratioArr[index - 1] : 0;

        if(ratioCur * restrictHeight > showerWidth){
            var end = ratioCur * restrictHeight - showerWidth > showerWidth - ratioLast * restrictHeight ? index - 1 : index;
            var height = Math.floor(showerWidth / (end == index ? ratioCur : ratioLast)) + 1;
            var totalWidth = 0, widthArr = [], totalWeight = 0; weightArr = [];

            console.log("begin: " + begin + ", end: " + end);
            for(var iter = begin; iter <= end; iter++){
                //var weight = ratioArr[iter] * 100 / obj[iter].height;
                var weight = ratioArr[iter] / Math.sqrt(obj[iter].height);

                weightArr.push({index: iter, weight: weight});
                widthArr[iter] = Math.round(height * ratioArr[iter]);

                totalWidth += widthArr[iter];
                totalWeight += weight;
            }
            weightArr.sort(function(obj1, obj2){return obj2.weight - obj1.weight;});

            var len = showerWidth - totalWidth;
            var gap = len;
            do{
                for(var iter in weightArr){
                    var x = Math.round(len * weightArr[iter].weight / totalWeight);
                    
                    x = x ? x : -1;
                    if(x < gap) x = gap;
                    //console.log(weightArr[iter].weight + "/" + totalWeight);
                    //console.log("x: " + x + "index: " + weightArr[iter].index);
                    widthArr[weightArr[iter].index] += x;
                    gap -= x;
                    if(gap == 0) break;
                }
            }while(gap);

            for(var iter = begin; iter <= end; iter++){
                imageShower.appendChild(makeDiv(count + iter, obj[iter].path, widthArr[iter], height, obj[iter].width, obj[iter].height));
            }

            index = end;
            begin = end + 1;
            ratioCur = 0;
        }
    }
    oldObj = obj.slice(begin);

    if(window.innerHeight == document.body.scrollHeight) genEmiter();
}

function adjEmiter(){
    if(oldHeight != window.innerHeight){
        var fix_layer = document.getElementById("fix_layer");

        oldHeight = window.innerHeight;

        if(fix_layer.style.height != "0px"){
            var w = document.body.offsetWidth;
            var h = window.innerHeight;
            var fix_bg = document.getElementById("fix_bg");
            var layer = document.getElementById("layer");
            var index = parseInt(layer.getAttribute("index"), 10);
            var image = document.getElementById("image_shower").children[index];
            var width = parseInt(image.getAttribute("hw"), 10);
            var height = parseInt(image.getAttribute("hh"), 10);

            fix_bg.style.height = h + "px";
            fix_layer.style.height = h + "px";

            if(w / h > width / height){
                if(height > h) layer.height = h;
            }else if(width > w){
                layer.width = w;
            }
        }
    }


    if(oldWidth != document.body.offsetWidth){
        oldTime = time();
        oldWidth = document.body.offsetWidth;
        setTimeout(function(){
                    if(time() - oldTime >= 500){
                        adjust();
                    }
                }, 500);
    }
}

function adjust(){
    var obj = [];
    var imageShower = document.getElementById("image_shower");
    var children = imageShower.children;
    
    for(var index = 0; index < children.length; index++){
        //console.log("index: " + index + ", " + children[index]);
        var width = parseInt(children[index].getAttribute("hw"), 10);
        var height = parseInt(children[index].getAttribute("hh"), 10);

        obj.push({width: width, height: height, path: children[index].firstElementChild.src});
    }
    
    imageShower.innerHTML = "";
    obj = obj.concat(oldObj);
    oldObj = [];
    gen(obj);
}

genEmiter()
