var mongoose = require('mongoose');

var BonesScheme = mongoose.Schema(
                    {
                    	Step   	   : String,
                    	Velocity     : String,
                         ParamKey     : String,

                    	Shoulder_L   : String, // x|y|z
                    	Arm_L	   : String, // x|y|z

                    	Thigh_L 	   : String, // t|b|a
                    	Calve_L      : String, // t|b|a
                         Angkle_L     : String, //sholder|arm

                         Shoulder_R   : String,
                         Arm_R        : String, //thigh|calves|ankle

                         Thigh_R      : String,
                         Calve_R      : String, //thigh|calves|ankle
                         Angkle_R     : String
                    },
                    {
                        timestamps: true
                    }
);
module.exports = mongoose.model('BonesMotionParams', BonesScheme);

