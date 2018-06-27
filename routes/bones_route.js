module.exports = function(app) {

    var bones = require('../controllers/bones_controller.js');

    // Create a new Note
    app.post('/bones', bones.create);

    // Retrieve all Notes
    app.get('/bones/:FunctionType', bones.findAll);

    // Retrieve a single Note with boneId
    app.get('/bones/:FunctionType/:ParamKey', bones.findOne);

    // Update a Note with boneId
    app.put('/bones/:ParamKey', bones.update);

    // Delete a Note with boneId
    app.delete('/bones/:ParamKey', bones.delete);
}