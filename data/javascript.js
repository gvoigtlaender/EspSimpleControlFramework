var x=null,lt,to,tp,pc='';
function eb(s){
    return document.getElementById(s);
}
function qs(s){
    return document.querySelector(s);
}
function sp(i){
    eb(i).type=(eb(i).type==='text'?'password':'text');
}
function wl(f){
    window.addEventListener('load',f);
}
var ft;
function la(p){
    a=p||'';
    clearTimeout(ft);
    clearTimeout(lt);
    if(x!=null){
        x.abort()
    }
    x=new XMLHttpRequest();
    x.onreadystatechange=function(){
        if(x.readyState==4&&x.status==200){
            var s=x.responseText.replace(/{t}/g,"<table style='width:100%'>").replace(/{s}/g,"<tr><th>").replace(/{m}/g,"</th><td style='width:20px;white-space:nowrap'>").replace(/{e}/g,"</td></tr>");
            eb('l1').innerHTML=s;
            clearTimeout(ft);
            clearTimeout(lt);
            lt=setTimeout(la,2345);
        }
    };
    x.open('GET','.?m=1'+a,true);
    x.send();
    ft=setTimeout(la,20000);
}
function sw(p){
    a=p||'';
    clearTimeout(ft);
    clearTimeout(lt);
    if(x!=null){
        x.abort()
    }
    x=new XMLHttpRequest();
    x.onreadystatechange=function(){
        if(x.readyState==4&&x.status==200){
            var s=x.responseText.replace(/{t}/g,"<table style='width:100%'>").replace(/{s}/g,"<tr><th>").replace(/{m}/g,"</th><td style='width:20px;white-space:nowrap'>").replace(/{e}/g,"</td></tr>");
            eb('l1').innerHTML=s;
            clearTimeout(ft);
            clearTimeout(lt);
            lt=setTimeout(la,2345);
        }
    };
    x.open('GET','switch?m=1'+a,true);
    x.send();
    ft=setTimeout(la,20000);
}
function lc(v,i,p){
    if(eb('s')){
        if(v=='h'||v=='d'){
            var sl=eb('sl4').value;
            eb('s').style.background='linear-gradient(to right,rgb('+sl+'%,'+sl+'%,'+sl+'%),hsl('+eb('sl2').value+',100%,50%))';
        }
    }
    la('&'+v+i+'='+p);
}
wl(la);
function jd(){
    var t=0,i=document.querySelectorAll('input,button,textarea,select');
    while(i.length>=t){
        if(i[t]){
            i[t]['name']=(i[t].hasAttribute('id')&&(!i[t].hasAttribute('name')))?i[t]['id']:i[t]['name'];
        }
        t++;
    }
}
wl(jd);