from pymongo import MongoClient
from bson.objectid import ObjectId
class AnimalShelter(object):
    def __init__(self, username, password):
         # Initializes mongoclient to access database.
         self.client = MongoClient('mongodb://%s:%s@localhost:46590' % (username, password))
         self.database = self.client['AAC']
     # Create function creates a new document and takes in a dictonary.
     # It returns true if a document is created, but false if it doesn't.
    def create(self, data):
        if data is not None:
            self.database.animals.insert(data)
            return True
        else:
            raise Exception("Nothing to save, because data parameter is empty")
            return False
     # Read function takes in a paramater and returns a corresponding item.
    def read(self, param):
        if param is not None:
            # Looks for a document based on the parameters.
            data = self.database.animals.find(param)
        else:
            data = self.database.animals.find({})
        return data
    # Update function recieves a query and record
    def update(self, query, record):
        # If the record exists, then update.
        if record is not None:
            # update the documents matching the query criteria and print no. of documents updated in json format
            update_result = self.database.animals.update_many(query, record)
            print("Updated")
            return update_result
        else:
            raise Exception("Record not found")
     # Delete function that takes data
    def delete(self, data):
         # Verify that there is a record to be deleted.
        if data is not None:
             # delete the documents matching data criteria and print confirmation
            delete_result = self.database.animals.delete_many(data)
            print("Deleted")
            return delete_result
        else:
             raise Exception("No record found.")
