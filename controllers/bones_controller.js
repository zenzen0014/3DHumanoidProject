var Bones = require('../models/bones_model.js');

exports.create = function(req, res) {
    var bone = new Bones({
        Step        : req.body.Step,
        Velocity    : req.body.Velocity,
        ParamKey    : req.body.ParamKey,
        Shoulder_L  : req.body.Shoulder_L,
        Arm_L       : req.body.Arm_L,
        Thigh_L     : req.body.Thigh_L,
        Calve_L     : req.body.Calve_L,
        Angkle_L    : req.body.Angkle_L,
        Shoulder_R  : req.body.Shoulder_R,
        Arm_R       : req.body.Arm_R,
        Thigh_R     : req.body.Thigh_R,
        Calve_R     : req.body.Calve_R,
        Angkle_R    : req.body.Angkle_R
    });

    bone.save(function(err, data) {
        if(err) {
            console.log(err);
            res.status(500).send({message: "Some error occurred while creating the Note."});
        } else {
            res.send(data);
        }
    });
};




exports.findAll = function(req, res) {
	// Retrieve and return all notes from the database.
    if(req.params.FunctionType == "all"){
        Bones.find(function(err, notes){
            if(err) {
                console.log(err);
                res.status(500).send({message: "Some error occurred while retrieving notes."});
            } else {
                res.send(notes);
            }
        });
    }else if(req.params.FunctionType == "key"){
        Bones.aggregate([{
                    $group : {
                        _id : "$ParamKey",
                        count: { $sum: 1 } 
                    }
        }], function(err, notes){
            if(err) {
                console.log(err);
                res.status(500).send({message: "Some error occurred while retrieving notes."});
            } else {
                res.send(notes);
            }
        });
    }
    
};





exports.findOne = function(req, res) {
    // Find a single note with a boneId

    if(req.params.FunctionType == "key"){
        Bones.findOne({ParamKey: req.params.ParamKey, Step: 0}, function(err, data) {
            if(err) {
                console.log(err);
                if(err.kind === 'ObjectId') {
                    return res.status(404).send({message: "Note not found with id " + req.params.ParamKey});                
                }
                return res.status(500).send({message: "Error retrieving note with id " + req.params.ParamKey});
            } 

            if(!data) {
                return res.status(404).send({message: "Note not found with id " + req.params.ParamKey});            
            }

            res.send(data);
        });
    }else if(req.params.FunctionType == "all"){
        Bones.find({ParamKey: req.params.ParamKey}, function(err, data) {
            if(err) {
                console.log(err);
                if(err.kind === 'ObjectId') {
                    return res.status(404).send({message: "Note not found with id " + req.params.ParamKey});                
                }
                return res.status(500).send({message: "Error retrieving note with id " + req.params.ParamKey});
            } 

            if(!data) {
                return res.status(404).send({message: "Note not found with id " + req.params.ParamKey});            
            }

            res.send(data);
        });
    }
    
};


















exports.update = function(req, res) {
	// Update a note identified by the boneId in the request
    Bones.findById(req.params.ParamKey, function(err, note) {
        if(err) {
            console.log(err);
            if(err.kind === 'ObjectId') {
                return res.status(404).send({message: "Note not found with id " + req.params.ParamKey});                
            }
            return res.status(500).send({message: "Error finding note with id " + req.params.ParamKey});
        }

        if(!note) {
            return res.status(404).send({message: "Note not found with id " + req.params.ParamKey});            
        }

        note.title = req.body.title;
        note.content = req.body.content;

        note.save(function(err, data){
            if(err) {
                res.status(500).send({message: "Could not update note with id " + req.params.ParamKey});
            } else {
                res.send(data);
            }
        });
    });
};





exports.delete = function(req, res) {
    // Delete a note with the specified boneId in the request
    Bones.findByIdAndRemove(req.params.boneId, function(err, note) {
        if(err) {
            console.log(err);
            if(err.kind === 'ObjectId') {
                return res.status(404).send({message: "Note not found with id " + req.params.ParamKey});                
            }
            return res.status(500).send({message: "Could not delete note with id " + req.params.ParamKey});
        }

        if(!note) {
            return res.status(404).send({message: "Note not found with id " + req.params.ParamKey});
        }

        res.send({message: "Note deleted successfully!"})
    });
};