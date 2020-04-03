var Temp = msg.payload.Temp;
var Humid = msg.payload.Humid;

msg.url = "https://notify-api.line.me/api/notify";
var AccessToken = "tjdsdcChlZFN3rHpruos4ivN7JgWaEqaKU8eUtlhkPe";
msg.headers = {'content-type':'application/x-www-form-urlencoded','Authorization':'Bearer '+ AccessToken};

if(Temp >= 50){
    msg.payload = {
        "message":"Temperature:" + Temp + " is over the limit!",
        "stickerPackageId" : "4",
        "stickerId" : "274"
    }
}

if(Humid >= 50){
    msg.payload = {
        "message":"Humidity:" + Humid + " is over the limit!",
        "stickerPackageId" : "4",
        "stickerId" : "266"
    }
}

return msg;