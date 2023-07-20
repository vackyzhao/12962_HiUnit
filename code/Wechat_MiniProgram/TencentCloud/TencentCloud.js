const app = getApp()
const util = require('../../utils/util.js')
Page({
  data: {
    productId: app.globalData.productId,
    deviceName: app.globalData.deviceName,
    stateReported: {},

    //第一个摇杆
    StartX1:'50',
    StartY1:'380',
    leftLooks1: '',
    topLooks1: '',
    //半径
    radius1: '40',
    speed1: '0',
    angle1: '',

    //第二个摇杆
    StartX2:'190',
    StartY2:'380',
    leftLooks2: '',
    topLooks2: '',
    //半径
    radius2: '40',
    resetButton: false,
  }, 

  onLoad: function (options) {
    console.log("index onLoad")
    if (!app.globalData.productId) {
      wx.showToast({
        title: "产品ID不能为空",
        icon: 'none',
        duration: 3000
      })
      return
    } else if (!app.globalData.deviceName) {
      wx.showToast({
        title: "设备名称不能为空",
        icon: 'none',
        duration: 3000
      })
      return
    }
    // this.update()
  },
  update() {
    wx.showLoading()
    wx.cloud.callFunction({
      name: 'iothub-shadow-query',
      data: {
        ProductId: app.globalData.productId,
        DeviceName: app.globalData.deviceName,
        SecretId: app.globalData.secretId,
        SecretKey: app.globalData.secretKey,
      },
      success: res => {
        wx.showToast({
          icon: 'none',
          title: 'Subscribe完成，获取云端数据成功',
        })
        let deviceData = JSON.parse(res.result.Data)

        this.setData({
          stateReported: deviceData.payload.state.reported
        })
        console.log("result:", deviceData)
      },
      fail: err => {
        wx.showToast({
          icon: 'none',
          title: 'Subscribe失败，获取云端数据失败',
        })
        console.error('[云函数] [iotexplorer] 调用失败：', err)
      }
    })
  },
//   switchChange_master
switchChange_master(e) {
    let value_master = 0
    if (e.detail.value == true) {
        value_master = 1
    }
    let item = e.currentTarget.dataset.item
    let obj = {
      [`${item}`]: value_master
    }
    let payload = JSON.stringify(obj)
    JSON.parse
    console.log(payload)
    wx.showLoading()
    wx.cloud.callFunction({
      name: 'iothub-publish',
      data: {
        SecretId: app.globalData.secretId,
        SecretKey: app.globalData.secretKey,
        ProductId: app.globalData.productId,
        DeviceName: app.globalData.deviceName,
        Topic: app.globalData.productId + "/" + app.globalData.deviceName + "/data",
        Payload: payload,
      },
      success: res => {
        wx.showToast({
          icon: 'none',
          title: 'publish完成',
        })
        console.log("res:", res)
      },
      fail: err => {
        wx.showToast({
          icon: 'none',
          title: 'publish失败，请连接设备',
        })
        console.error('[云函数] [iotexplorer] 调用失败：', err)
      }
    })  
  },
  switchChange_mode(e) {
    let value_mode = 0
    if (e.detail.value == true) {
        value_mode = 1
    }
    let item = e.currentTarget.dataset.item
    let obj = {
      [`${item}`]: value_mode
    }
    let payload = JSON.stringify(obj)
    JSON.parse
    console.log(payload)
    wx.showLoading()
    wx.cloud.callFunction({
      name: 'iothub-publish',
      data: {
        SecretId: app.globalData.secretId,
        SecretKey: app.globalData.secretKey,
        ProductId: app.globalData.productId,
        DeviceName: app.globalData.deviceName,
        Topic: app.globalData.productId + "/" + app.globalData.deviceName + "/data",
        Payload: payload,
      },
      success: res => {
        wx.showToast({
          icon: 'none',
          title: 'publish完成',
        })
        console.log("res:", res)
      },
      fail: err => {
        wx.showToast({
          icon: 'none',
          title: 'publish失败，请连接设备',
        })
        console.error('[云函数] [iotexplorer] 调用失败：', err)
      }
    })  
  },
  switchChange_rod(e) {
    let value_rod = 0
    if (e.detail.value == true) {
      value_rod = 1
    }
    let item = e.currentTarget.dataset.item
    let obj = {
      [`${item}`]: value_rod
    }
    let payload = JSON.stringify(obj)
    JSON.parse
    console.log(payload)
    wx.showLoading()
    wx.cloud.callFunction({
      name: 'iothub-publish',
      data: {
        SecretId: app.globalData.secretId,
        SecretKey: app.globalData.secretKey,
        ProductId: app.globalData.productId,
        DeviceName: app.globalData.deviceName,
        Topic: app.globalData.productId + "/" + app.globalData.deviceName + "/data",
        Payload: payload,
      },
      success: res => {
        wx.showToast({
          icon: 'none',
          title: 'publish完成',
        })
        console.log("res:", res)
      },
      fail: err => {
        wx.showToast({
          icon: 'none',
          title: 'publish失败，请连接设备',
        })
        console.error('[云函数] [iotexplorer] 调用失败：', err)
      }
    })  
  },
  switchChange_pen(e) {
    let value_pen = 0
    if (e.detail.value == true) {
      value_pen = 1
    }
    let item = e.currentTarget.dataset.item
    let obj = {
      [`${item}`]: value_pen
    }
    let payload = JSON.stringify(obj)
    JSON.parse
    console.log(payload)
    wx.showLoading()
    wx.cloud.callFunction({
      name: 'iothub-publish',
      data: {
        SecretId: app.globalData.secretId,
        SecretKey: app.globalData.secretKey,
        ProductId: app.globalData.productId,
        DeviceName: app.globalData.deviceName,
        Topic: app.globalData.productId + "/" + app.globalData.deviceName + "/data",
        Payload: payload,
      },
      success: res => {
        wx.showToast({
          icon: 'none',
          title: 'publish完成',
        })
        console.log("res:", res)
      },
      fail: err => {
        wx.showToast({
          icon: 'none',
          title: 'publish失败，请连接设备',
        })
        console.error('[云函数] [iotexplorer] 调用失败：', err)
      }
    })  
  },

  //拖动摇杆移动
  //第一个摇杆
    ImageTouchMove1: function (e) {
        var self1 = this;
        var touchX1 = e.touches[0].clientX - 40;
        var touchY1 = e.touches[0].clientY - 40;
        var movePos1 = self1.GetPosition1(touchX1, touchY1);
        self1.setData({
            leftLooks1: movePos1.posX1,
            topLooks1: movePos1.posY1,
            speed1: movePos1.speedF,
        })
        this.publishPosition1();
    },

    GetPosition1: function (touchX1, touchY1) {
        var self1 = this;
        var DValue_X1;
        var Dvalue_Y1;
        var Dvalue_Z1;
        var imageX1;
        var imageY1;
        var ratio1;
        var SpeedValue1;
        DValue_X1 = touchX1 - self1.data.StartX1;
        Dvalue_Y1 = touchY1 - self1.data.StartY1;
        Dvalue_Z1 = Math.sqrt(DValue_X1 * DValue_X1 + Dvalue_Y1 * Dvalue_Y1);
        
        //触碰点在范围内
        if (Dvalue_Z1 <= self1.data.radius1) {
            imageX1 = touchX1;
            imageY1 = touchY1;
            imageX1 = Math.round(imageX1);
            imageY1 = Math.round(imageY1);
            SpeedValue1 = Math.sqrt((imageX1-50)**2 + (imageY1-380)**2);
            SpeedValue1 = Math.round(SpeedValue1*50);
        }
        //触碰点在范围外
        else {
            ratio1 = self1.data.radius1 / Dvalue_Z1;
            imageX1 = DValue_X1 * ratio1 + 50;
            imageY1 = Dvalue_Y1 * ratio1 + 380;
            imageX1 = Math.round(imageX1);
            imageY1 = Math.round(imageY1);
            SpeedValue1 = Math.sqrt((imageX1-50)**2 + (imageY1-380)**2);
            SpeedValue1 = Math.round(SpeedValue1*50);
        }
        self1.GetAngle1(imageX1-50, imageY1-380);
        return { posX1: imageX1, posY1: imageY1, speedF: SpeedValue1 };  
    },
    GetAngle1: function (DValue_Y1, Dvalue_X1) {
        // console.log(DValue_Y)
        var self = this;
        if (DValue_Y1 != 0) {
          var angleTan = Dvalue_X1 / DValue_Y1;
          var ang = Math.atan(angleTan);
    
          var angs = ang * 180 / 3.14;
          
          var result = Math.round(angs);

          if((DValue_Y1 >= 0) ){
                result = 90 + result
            }
            else if ((DValue_Y1 <= 0)){
                result = result - 90;
            }
            
          self.setData({
            angle1: result
          })
        }
    
    },
    ImageReturn1: function (e) {
        var self1 = this;
        self1.setData({
            leftLooks1: self1.data.StartX1,
            topLooks1: self1.data.StartY1,
            speed1: "0",
            angle1:"0"
        })
        this.publishReturn1();
    },

    //第二个摇杆
    ImageTouchMove2: function (e) {
        var self2 = this;
        var touchX2 = e.touches[0].clientX - 40;
        var touchY2 = e.touches[0].clientY - 40;
        var movePos2 = self2.GetPosition2(touchX2, touchY2);
        self2.setData({
            leftLooks2: movePos2.posX2,
            topLooks2: movePos2.posY2,
        })
        this.publishPosition2();
    },

    //第二个摇杆
    GetPosition2: function (touchX2, touchY2) {
        var self2 = this;
        var DValue_X2;
        var Dvalue_Y2;
        var Dvalue_Z2;
        var imageX2;
        var imageY2;
        var ratio2;
        DValue_X2 = touchX2 - self2.data.StartX2;
        Dvalue_Y2 = touchY2 - self2.data.StartY2;
        Dvalue_Z2 = Math.sqrt(DValue_X2 * DValue_X2 + Dvalue_Y2 * Dvalue_Y2);
        //触碰点在范围内
        if (Dvalue_Z2 <= self2.data.radius2) {
            imageX2 = touchX2;
            imageY2 = touchY2;
            imageX2 = Math.round(imageX2);
            imageY2 = Math.round(imageY2);
        }
        //触碰点在范围外
        else {
            ratio2 = self2.data.radius2 / Dvalue_Z2;
            imageX2 = DValue_X2 * ratio2 + 190;
            imageY2 = Dvalue_Y2 * ratio2 + 380;
            imageX2 = Math.round(imageX2);
            imageY2 = Math.round(imageY2);
        }
        return { posX2: imageX2, posY2: imageY2};
    },
    
    ImageReturn2: function (e) {
        var self2 = this;
        self2.setData({
            // leftLooks2: self2.data.StartX2,
            // topLooks2: self2.data.StartY2,
        })
    },
    resetPanTilt: function() {
        // 重置摇杆到原始位置
        this.setData({
          leftLooks2: this.data.StartX2,
          topLooks2: this.data.StartY2,
        });
        this.publishReturn2();
    },
    publishReturn1: function(){
        let obj = {
          speed1: this.data.speed1,
          angle1: this.data.angle1,
        }
        let payload = JSON.stringify(obj)
        console.log(payload)
        wx.showLoading()
        wx.cloud.callFunction({
          name: 'iothub-publish',
          data: {
            SecretId: app.globalData.secretId,
            SecretKey: app.globalData.secretKey,
            ProductId: app.globalData.productId,
            DeviceName: app.globalData.deviceName,
            Topic: app.globalData.productId + "/" + app.globalData.deviceName + "/data",
            Payload: payload,
          },
          success: res => {
            wx.showToast({
              icon: 'none',
              title: 'publish完成',
            })
            console.log("res:", res)
          },
          fail: err => {
            wx.showToast({
              icon: 'none',
              title: 'publish失败，请连接设备',
            })
            console.error('[云函数] [iotexplorer] 调用失败：', err)
          }
        })
    },
    publishReturn2: function(){
        let obj = {
            posx: this.data.leftLooks2 - 190,
            posy: this.data.topLooks2 - 380,
        }
        let payload = JSON.stringify(obj)
        console.log(payload)
        wx.showLoading()
        wx.cloud.callFunction({
          name: 'iothub-publish',
          data: {
            SecretId: app.globalData.secretId,
            SecretKey: app.globalData.secretKey,
            ProductId: app.globalData.productId,
            DeviceName: app.globalData.deviceName,
            Topic: app.globalData.productId + "/" + app.globalData.deviceName + "/data",
            Payload: payload,
          },
          success: res => {
            wx.showToast({
              icon: 'none',
              title: 'publish完成',
            })
            console.log("res:", res)
          },
          fail: err => {
            wx.showToast({
              icon: 'none',
              title: 'publish失败，请连接设备',
            })
            console.error('[云函数] [iotexplorer] 调用失败：', err)
          }
        })
    },
    publishPosition1: util.throttle(function() {
        
        let obj = {
            speed1: this.data.speed1,
            angle1: this.data.angle1,
            
        }
        let payload = JSON.stringify(obj)
        console.log(payload)
        wx.showLoading()
        wx.cloud.callFunction({
          name: 'iothub-publish',
          data: {
            SecretId: app.globalData.secretId,
            SecretKey: app.globalData.secretKey,
            ProductId: app.globalData.productId,
            DeviceName: app.globalData.deviceName,
            Topic: app.globalData.productId + "/" + app.globalData.deviceName + "/data",
            Payload: payload,
          },
          success: res => {
            wx.showToast({
              icon: 'none',
              title: 'publish完成',
            })
            console.log("res:", res)
          },
          fail: err => {
            wx.showToast({
              icon: 'none',
              title: 'publish失败，请连接设备',
            })
            console.error('[云函数] [iotexplorer] 调用失败：', err)
          }
        })  
      }),

    publishPosition2: util.throttle(function() {
        
        let obj = {
            posx: this.data.leftLooks2 - 190,
            posy: this.data.topLooks2 - 380,
        }
        let payload = JSON.stringify(obj)
        console.log(payload)
        wx.showLoading()
        wx.cloud.callFunction({
          name: 'iothub-publish',
          data: {
            SecretId: app.globalData.secretId,
            SecretKey: app.globalData.secretKey,
            ProductId: app.globalData.productId,
            DeviceName: app.globalData.deviceName,
            Topic: app.globalData.productId + "/" + app.globalData.deviceName + "/data",
            Payload: payload,
          },
          success: res => {
            wx.showToast({
              icon: 'none',
              title: 'publish完成',
            })
            console.log("res:", res)
          },
          fail: err => {
            wx.showToast({
              icon: 'none',
              title: 'publish失败，请连接设备',
            })
            console.error('[云函数] [iotexplorer] 调用失败：', err)
          }
        })  
      }),
    /**
   * 生命周期函数--监听页面显示
   */
  onShow: function () {
  
  },
  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload: function () {

  },
    /**
   * 页面相关事件处理函数--监听用户下拉动作
   */
  onPullDownRefresh: function () {

  },
  
  /**
   * 页面上拉触底事件的处理函数
   */
  onReachBottom: function () {

  },
})
